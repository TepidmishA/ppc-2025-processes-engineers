#include "perepelkin_i_qsort_batcher_oddeven_merge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <stack>
#include <tuple>
#include <utility>
#include <vector>

#include "perepelkin_i_qsort_batcher_oddeven_merge/common/include/common.hpp"

namespace perepelkin_i_qsort_batcher_oddeven_merge {

PerepelkinIQsortBatcherOddEvenMergeMPI::PerepelkinIQsortBatcherOddEvenMergeMPI(const InType &in) {
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num_);

  SetTypeOfTask(GetStaticTypeOfTask());
  if (proc_rank_ == 0) {
    GetInput() = in;
  }
  GetOutput() = std::vector<double>();
}

bool PerepelkinIQsortBatcherOddEvenMergeMPI::ValidationImpl() {
  return GetOutput().empty();
}

bool PerepelkinIQsortBatcherOddEvenMergeMPI::PreProcessingImpl() {
  return true;
}

bool PerepelkinIQsortBatcherOddEvenMergeMPI::RunImpl() {
  // [1] Broadcast original and padded sizes
  size_t original_size = 0;
  size_t padded_size = 0;

  BcastSizes(original_size, padded_size);

  if (original_size == 0) {
    return true;
  }

  // [2] Distribute data
  std::vector<double> padded_input;
  if (proc_rank_ == 0) {
    padded_input = GetInput();
    if (padded_size > original_size) {
      padded_input.resize(padded_size, std::numeric_limits<double>::infinity());
    }
  }

  std::vector<int> counts;
  std::vector<int> displs;
  std::vector<double> local_data;
  DistributeData(padded_size, padded_input, counts, displs, local_data);

  // [3] Local sort
  std::qsort(local_data.data(), local_data.size(), sizeof(double), [](const void *a, const void *b) {
    double arg1 = *static_cast<const double *>(a);
    double arg2 = *static_cast<const double *>(b);
    if (arg1 < arg2) {
      return -1;
    }
    if (arg1 > arg2) {
      return 1;
    }
    return 0;
  });

  // [4] Global merge via comparator network
  std::vector<std::pair<int, int>> comparators;
  BuildComparators(comparators);
  ProcessComparators(counts, local_data, comparators);

  // [5] Gather result on root
  std::vector<double> gathered;
  if (proc_rank_ == 0) {
    gathered.resize(padded_size);
  }

  MPI_Gatherv(local_data.data(), static_cast<int>(local_data.size()), MPI_DOUBLE, gathered.data(), counts.data(),
              displs.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if (proc_rank_ == 0) {
    gathered.resize(original_size);
    GetOutput() = std::move(gathered);
  }

  // [6] Bcast output to all processes
  GetOutput().resize(original_size);
  MPI_Bcast(GetOutput().data(), static_cast<int>(original_size), MPI_DOUBLE, 0, MPI_COMM_WORLD);

  return true;
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::BcastSizes(size_t &original_size, size_t &padded_size) {
  if (proc_rank_ == 0) {
    original_size = GetInput().size();
    const size_t remainder = original_size % proc_num_;
    padded_size = original_size + (remainder == 0 ? 0 : (proc_num_ - remainder));
  }

  MPI_Bcast(&original_size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
  MPI_Bcast(&padded_size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::DistributeData(const size_t &padded_size,
                                                            const std::vector<double> &padded_input,
                                                            std::vector<int> &counts, std::vector<int> &displs,
                                                            std::vector<double> &local_data) const {
  const int base_size = static_cast<int>(padded_size / proc_num_);

  counts.resize(proc_num_);
  displs.resize(proc_num_);

  for (int i = 0, offset = 0; i < proc_num_; i++) {
    counts[i] = base_size;
    displs[i] = offset;
    offset += base_size;
  }

  const int local_size = counts[proc_rank_];
  local_data.resize(local_size);

  MPI_Scatterv(padded_input.data(), counts.data(), displs.data(), MPI_DOUBLE, local_data.data(), local_size, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildComparators(std::vector<std::pair<int, int>> &comparators) const {
  std::vector<int> procs(proc_num_);
  for (int i = 0; i < proc_num_; i++) {
    procs[i] = i;
  }

  BuildStageB(procs, comparators);
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildStageB(const std::vector<int> &procs,
                                                         std::vector<std::pair<int, int>> &comparators) {
  // Task: (subarray, is_merge_phase)
  std::stack<std::pair<std::vector<int>, bool>> tasks;
  tasks.emplace(procs, false);

  while (!tasks.empty()) {
    auto [current, is_merge] = tasks.top();
    tasks.pop();

    if (current.size() <= 1) {
      continue;
    }

    // Split phase: divide and schedule children
    auto mid = static_cast<DiffT>(current.size() / 2);
    std::vector<int> left(current.begin(), current.begin() + mid);
    std::vector<int> right(current.begin() + mid, current.end());

    if (is_merge) {
      BuildStageS(left, right, comparators);
      continue;
    }

    // Schedule merge after children complete
    tasks.emplace(current, true);
    tasks.emplace(right, false);  // Right child first (LIFO order)
    tasks.emplace(left, false);
  }
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildStageS(const std::vector<int> &up, const std::vector<int> &down,
                                                         std::vector<std::pair<int, int>> &comparators) {
  // Task: (part_up, part_down, is_merge_phase)
  std::stack<std::tuple<std::vector<int>, std::vector<int>, bool>> tasks;
  tasks.emplace(up, down, false);

  while (!tasks.empty()) {
    auto [part_up, part_down, is_merge] = tasks.top();
    tasks.pop();
    const size_t total_size = part_up.size() + part_down.size();

    if (total_size <= 1) {
      continue;
    }
    if (total_size == 2) {
      comparators.emplace_back(part_up[0], part_down[0]);
      continue;
    }

    if (!is_merge) {
      // Split phase: separate odd/even indices
      auto [a_odd, a_even] = Split(part_up);
      auto [b_odd, b_even] = Split(part_down);

      // Schedule merge after recursive processing
      tasks.emplace(part_up, part_down, true);
      tasks.emplace(a_even, b_even, false);
      tasks.emplace(a_odd, b_odd, false);
      continue;
    }

    // Merge phase: connect adjacent elements
    std::vector<int> merged;
    merged.reserve(total_size);
    merged.insert(merged.end(), part_up.begin(), part_up.end());
    merged.insert(merged.end(), part_down.begin(), part_down.end());

    for (size_t i = 1; i < merged.size() - 1; i += 2) {
      comparators.emplace_back(merged[i], merged[i + 1]);
    }
  }
}

std::pair<std::vector<int>, std::vector<int>> PerepelkinIQsortBatcherOddEvenMergeMPI::Split(
    const std::vector<int> &data) {
  std::vector<int> odd;
  std::vector<int> even;
  for (size_t i = 0; i < data.size(); i++) {
    if (i % 2 == 0) {
      even.push_back(data[i]);
    } else {
      odd.push_back(data[i]);
    }
  }
  return std::make_pair(std::move(odd), std::move(even));
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::ProcessComparators(
    const std::vector<int> &counts, std::vector<double> &local_data,
    const std::vector<std::pair<int, int>> &comparators) const {
  std::vector<double> peer_buffer;
  std::vector<double> temp;

  for (const auto &comp : comparators) {
    const int first = comp.first;
    const int second = comp.second;

    if (proc_rank_ != first && proc_rank_ != second) {
      continue;
    }

    const int peer = (proc_rank_ == first) ? second : first;
    const int local_size = counts[proc_rank_];
    const int peer_size = counts[peer];

    peer_buffer.resize(peer_size);
    temp.resize(local_size);

    MPI_Status status;
    MPI_Sendrecv(local_data.data(), local_size, MPI_DOUBLE, peer, 0, peer_buffer.data(), peer_size, MPI_DOUBLE, peer, 0,
                 MPI_COMM_WORLD, &status);

    MergeBlocks(local_data, peer_buffer, temp, proc_rank_ == first);
    local_data.swap(temp);
  }
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::MergeBlocks(const std::vector<double> &local_data,
                                                         const std::vector<double> &peer_buffer,
                                                         std::vector<double> &temp, bool keep_lower) {
  const int local_size = static_cast<int>(local_data.size());
  const int peer_size = static_cast<int>(peer_buffer.size());

  if (keep_lower) {
    for (int tmp_index = 0, res_index = 0, cur_index = 0; tmp_index < local_size; tmp_index++) {
      const double result = local_data[res_index];
      const double current = peer_buffer[cur_index];
      if (result < current) {
        temp[tmp_index] = result;
        res_index++;
      } else {
        temp[tmp_index] = current;
        cur_index++;
      }
    }
  } else {
    for (int tmp_index = local_size - 1, res_index = local_size - 1, cur_index = peer_size - 1; tmp_index >= 0;
         tmp_index--) {
      const double result = local_data[res_index];
      const double current = peer_buffer[cur_index];
      if (result > current) {
        temp[tmp_index] = result;
        res_index--;
      } else {
        temp[tmp_index] = current;
        cur_index--;
      }
    }
  }
}

bool PerepelkinIQsortBatcherOddEvenMergeMPI::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_qsort_batcher_oddeven_merge
