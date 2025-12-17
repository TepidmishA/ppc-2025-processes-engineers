#include "ovchinnikov_m_bubble_sort/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <vector>

#include "ovchinnikov_m_bubble_sort/common/include/common.hpp"

namespace ovchinnikov_m_bubble_sort {

namespace {

void SortOddEven(std::vector<int> &array) {
  if (array.empty()) {
    return;
  }
  bool sorted = false;
  while (!sorted) {
    sorted = true;
    for (size_t i = 0; i + 1 < array.size(); i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }
    for (size_t i = 1; i + 1 < array.size(); i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }
  }
}

int FindPartner(int rank, int phase) {
  int partner = 0;
  if (phase % 2 == 0) {
    if (rank % 2 == 0) {
      partner = rank + 1;
    } else {
      partner = rank - 1;
    }
  } else {
    if (rank % 2 == 0) {
      partner = rank - 1;
    } else {
      partner = rank + 1;
    }
  }
  return partner;
}

std::vector<int> SwapDataWithPartner(int partner, const std::vector<int> &local_data) {
  // Exchange sizes first
  int my_size = static_cast<int>(local_data.size());
  int partner_size = 0;

  MPI_Sendrecv(&my_size, 1, MPI_INT, partner, 0, &partner_size, 1, MPI_INT, partner, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

  // Exchange data
  std::vector<int> partner_data(partner_size);
  MPI_Sendrecv(local_data.data(), my_size, MPI_INT, partner, 1, partner_data.data(), partner_size, MPI_INT, partner, 1,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  return partner_data;
}

std::vector<int> MergeAndSplit(int rank, int partner, const std::vector<int> &local_data,
                               const std::vector<int> &partner_data) {
  std::vector<int> merged;
  merged.reserve(local_data.size() + partner_data.size());
  size_t i = 0;
  size_t j = 0;
  while (i < local_data.size() && j < partner_data.size()) {
    if (local_data[i] <= partner_data[j]) {
      merged.push_back(local_data[i]);
      i++;
    } else {
      merged.push_back(partner_data[j]);
      j++;
    }
  }
  while (i < local_data.size()) {
    merged.push_back(local_data[i]);
    i++;
  }
  while (j < partner_data.size()) {
    merged.push_back(partner_data[j]);
    j++;
  }
  // std::ranges::merge(local_data.begin(), local_data.end(), partner_data.begin(), partner_data.end(), merged.begin());

  std::vector<int> result(local_data.size());
  if (rank < partner) {
    // Keep smaller half
    std::copy(merged.begin(), merged.begin() + static_cast<int>(local_data.size()), result.begin());
  } else {
    // Keep larger half
    std::copy(merged.end() - static_cast<int>(local_data.size()), merged.end(), result.begin());
  }

  return result;
}

void ManageOddEven(int rank, int proc_count, std::vector<int> &local_data) {
  for (int phase = 0; phase < (proc_count + 1); phase++) {
    int partner = FindPartner(rank, phase);

    if (partner >= 0 && partner < proc_count) {  // partner validation
      std::vector<int> partner_data = SwapDataWithPartner(partner, local_data);
      local_data = MergeAndSplit(rank, partner, local_data, partner_data);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

}  // namespace

OvchinnikovMBubbleSortMPI::OvchinnikovMBubbleSortMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  static_cast<void>(GetOutput());
}

bool OvchinnikovMBubbleSortMPI::ValidationImpl() {
  return true;
}

bool OvchinnikovMBubbleSortMPI::PreProcessingImpl() {
  return true;
}

bool OvchinnikovMBubbleSortMPI::RunImpl() {
  int rank = 0;
  int proc_count = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_count);

  const auto &input_vector = GetInput();
  int vec_size = static_cast<int>(input_vector.size());

  if (vec_size == 0) {
    GetOutput() = std::vector<int>();
    return true;
  }

  std::vector<int> elem_count(proc_count);
  std::vector<int> elem_offset(proc_count);

  if (rank == 0) {
    int chunk_base = vec_size / proc_count;
    int chunk_extra = vec_size % proc_count;
    int offset = 0;

    for (int i = 0; i < proc_count; i++) {
      int chunk_end_bit = static_cast<int>(i < chunk_extra);
      elem_count[i] = chunk_base + chunk_end_bit;
      elem_offset[i] = offset;
      offset += elem_count[i];
    }
    //
  }

  MPI_Bcast(elem_count.data(), proc_count, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(elem_offset.data(), proc_count, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> local_data(elem_count[rank]);
  MPI_Scatterv(rank == 0 ? input_vector.data() : nullptr, elem_count.data(), elem_offset.data(), MPI_INT,
               local_data.data(), elem_count[rank], MPI_INT, 0, MPI_COMM_WORLD);

  SortOddEven(local_data);
  ManageOddEven(rank, proc_count, local_data);
  std::vector<int> result;
  if (rank == 0) {
    result.resize(vec_size);
  }

  MPI_Gatherv(local_data.data(), static_cast<int>(local_data.size()), MPI_INT, rank == 0 ? result.data() : nullptr,
              elem_count.data(), elem_offset.data(), MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    GetOutput() = result;
  } else {
    GetOutput().resize(vec_size);
  }

  MPI_Bcast(GetOutput().data(), vec_size, MPI_INT, 0, MPI_COMM_WORLD);
  return true;
}

bool OvchinnikovMBubbleSortMPI::PostProcessingImpl() {
  return true;
}

}  // namespace ovchinnikov_m_bubble_sort
