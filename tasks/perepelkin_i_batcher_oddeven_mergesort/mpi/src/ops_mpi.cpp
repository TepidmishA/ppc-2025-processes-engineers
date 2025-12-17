#include "perepelkin_i_batcher_oddeven_mergesort/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <numeric>
#include <vector>

#include "perepelkin_i_batcher_oddeven_mergesort/common/include/common.hpp"

namespace perepelkin_i_batcher_oddeven_mergesort {

PerepelkinIBatcherOddEvenMergeSortMPI::PerepelkinIBatcherOddEvenMergeSortMPI(const InType &in) {
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num_);

  SetTypeOfTask(GetStaticTypeOfTask());
  if (proc_rank_ == 0) {
    GetInput() = in;
  }
  GetOutput() = std::vector<double>();
}

bool PerepelkinIBatcherOddEvenMergeSortMPI::ValidationImpl() {
  return GetOutput().empty();
}

bool PerepelkinIBatcherOddEvenMergeSortMPI::PreProcessingImpl() {
  return true;
}

bool PerepelkinIBatcherOddEvenMergeSortMPI::RunImpl() {
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

  // [3] Each process sorts its local data
  if (local_data.size() > 1) {
    OddEvenMergeSort(local_data, 0, local_data.size());
  }

  // [4] Gather sorted data at root and perform final merge
  std::vector<double> gathered;
  if (proc_rank_ == 0) {
    gathered.resize(padded_size);
  }

  MPI_Gatherv(local_data.data(), local_data.size(), MPI_DOUBLE, gathered.data(), counts.data(), displs.data(), MPI_DOUBLE, 0,
              MPI_COMM_WORLD);

  if (proc_rank_ == 0) {
    OddEvenMergeSort(gathered, 0, gathered.size());
    gathered.resize(original_size);
    GetOutput() = gathered;   // TODO try std::move
  }

  // [5] Bcast output to all processes
  GetOutput().resize(original_size);
  MPI_Bcast(GetOutput().data(), original_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  return true;
}

void PerepelkinIBatcherOddEvenMergeSortMPI::BcastSizes(size_t &original_size, size_t &padded_size) {
  if (proc_rank_ == 0) {
    original_size = GetInput().size();
    padded_size = NextPowerOfTwo(original_size);
  }

  MPI_Bcast(&original_size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
  MPI_Bcast(&padded_size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
}

size_t PerepelkinIBatcherOddEvenMergeSortMPI::NextPowerOfTwo(const size_t &value) {
  size_t result = 1;
  while (result < value) {
    result <<= 1U;
  }
  return result;
}

void PerepelkinIBatcherOddEvenMergeSortMPI::DistributeData(const size_t &padded_size, const std::vector<double> &padded_input,
                                                           std::vector<int> &counts, std::vector<int> &displs,
                                                           std::vector<double> &local_data) {
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

  MPI_Scatterv(padded_input.data(), counts.data(), displs.data(), MPI_DOUBLE, local_data.data(), local_size, MPI_DOUBLE, 0,
               MPI_COMM_WORLD);
}

void PerepelkinIBatcherOddEvenMergeSortMPI::OddEvenMergeSort(std::vector<double> &data, const size_t left, const size_t size) {
  if (size <= 1) {
    return;
  }

  const size_t mid = size / 2;
  OddEvenMergeSort(data, left, mid);
  OddEvenMergeSort(data, left + mid, mid);
  OddEvenMerge(data, left, size, 1);
}

void PerepelkinIBatcherOddEvenMergeSortMPI::OddEvenMerge(std::vector<double> &data, const size_t left, const size_t size, const size_t gap) {
  const size_t step = gap * 2;
  if (step < size) {
    OddEvenMerge(data, left, size, step);
    OddEvenMerge(data, left + gap, size, step);
    for (size_t i = left + gap; i + gap < left + size; i += step) {
      if (data[i] > data[i + gap]) {
        std::swap(data[i], data[i + gap]);
      }
    }
  } else {
    if (data[left] > data[left + gap]) {
      std::swap(data[left], data[left + gap]);
    }
  }
}

bool PerepelkinIBatcherOddEvenMergeSortMPI::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_batcher_oddeven_mergesort
