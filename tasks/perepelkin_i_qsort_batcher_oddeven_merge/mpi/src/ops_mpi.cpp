#include "perepelkin_i_qsort_batcher_oddeven_merge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <numeric>
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
  if (!local_data.empty()) {
    std::ranges::sort(local_data.begin(), local_data.end());
  }

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
  const int remainder = static_cast<int>(padded_size % proc_num_);

  counts.resize(proc_num_);
  displs.resize(proc_num_);

  for (int i = 0, offset = 0; i < proc_num_; ++i) {
    counts[i] = base_size + (i < remainder ? 1 : 0);
    displs[i] = offset;
    offset += counts[i];
  }

  const int local_size = counts[proc_rank_];
  local_data.resize(local_size);

  MPI_Scatterv(padded_input.data(), counts.data(), displs.data(), MPI_DOUBLE, local_data.data(), local_size, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildComparators(std::vector<std::pair<int, int>> &comparators) const {
  std::vector<int> procs(proc_num_);
  std::ranges::iota(procs.begin(), procs.end(), 0);
  BuildStageB(procs, comparators);
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildStageS(const std::vector<int> &procs_up,
                                                         const std::vector<int> &procs_down,
                                                         std::vector<std::pair<int, int>> &comparators) const {
  const size_t proc_count = procs_up.size() + procs_down.size();
  if (proc_count == 1) {
    return;
  }
  if (proc_count == 2) {
    comparators.emplace_back(procs_up.front(), procs_down.front());
    return;
  }

  std::vector<int> procs_up_odd;
  std::vector<int> procs_up_even;
  std::vector<int> procs_down_odd;
  std::vector<int> procs_down_even;

  for (size_t i = 0; i < procs_up.size(); ++i) {
    if (i % 2 == 0) {
      procs_up_odd.push_back(procs_up[i]);
    } else {
      procs_up_even.push_back(procs_up[i]);
    }
  }

  for (size_t i = 0; i < procs_down.size(); ++i) {
    if (i % 2 == 0) {
      procs_down_odd.push_back(procs_down[i]);
    } else {
      procs_down_even.push_back(procs_down[i]);
    }
  }

  BuildStageS(procs_up_odd, procs_down_odd, comparators);
  BuildStageS(procs_up_even, procs_down_even, comparators);

  std::vector<int> merged;
  merged.reserve(procs_up.size() + procs_down.size());
  merged.insert(merged.end(), procs_up.begin(), procs_up.end());
  merged.insert(merged.end(), procs_down.begin(), procs_down.end());

  for (size_t i = 1; i + 1 < merged.size(); i += 2) {
    comparators.emplace_back(merged[i], merged[i + 1]);
  }
}

void PerepelkinIQsortBatcherOddEvenMergeMPI::BuildStageB(const std::vector<int> &procs,
                                                         std::vector<std::pair<int, int>> &comparators) const {
  if (procs.size() <= 1) {
    return;
  }

  const size_t mid = procs.size() / 2;
  std::vector<int> procs_up(procs.begin(), procs.begin() + static_cast<std::ptrdiff_t>(mid));
  std::vector<int> procs_down(procs.begin() + static_cast<std::ptrdiff_t>(mid), procs.end());

  BuildStageB(procs_up, comparators);
  BuildStageB(procs_down, comparators);
  BuildStageS(procs_up, procs_down, comparators);
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
