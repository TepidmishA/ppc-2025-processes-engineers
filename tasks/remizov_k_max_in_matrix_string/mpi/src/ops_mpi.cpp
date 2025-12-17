#include "remizov_k_max_in_matrix_string/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

#include "remizov_k_max_in_matrix_string/common/include/common.hpp"

namespace remizov_k_max_in_matrix_string {

RemizovKMaxInMatrixStringMPI::RemizovKMaxInMatrixStringMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  InType tmp(in);
  GetInput().swap(tmp);
}

bool RemizovKMaxInMatrixStringMPI::ValidationImpl() {
  return true;
}

bool RemizovKMaxInMatrixStringMPI::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

std::vector<int> RemizovKMaxInMatrixStringMPI::FindMaxValues(const int start, const int end) {
  if (start < 0) {
    return {};
  }

  const std::size_t input_size = GetInput().size();
  const int input_size_int = static_cast<int>(input_size);

  if (start >= input_size_int) {
    return {};
  }

  const int actual_end = (end >= input_size_int) ? input_size_int - 1 : end;
  const int num_rows = actual_end - start + 1;

  if (num_rows <= 0) {
    return {};
  }

  std::vector<int> result;
  result.reserve(static_cast<std::size_t>(num_rows));

  for (int i = start; i <= actual_end; ++i) {
    if (!GetInput()[i].empty()) {
      int max_val = std::numeric_limits<int>::min();
      for (const auto &val : GetInput()[i]) {
        max_val = std::max(val, max_val);
      }
      result.push_back(max_val);
    } else {
      result.push_back(std::numeric_limits<int>::min());
    }
  }

  return result;
}

std::vector<int> RemizovKMaxInMatrixStringMPI::CalculatingInterval(int size_prcs, int rank, int count_rows) {
  if (count_rows <= 0 || size_prcs <= 0 || rank < 0 || rank >= size_prcs) {
    return {-1, -1};
  }

  const int base_rows = count_rows / size_prcs;
  const int extra_rows = count_rows % size_prcs;

  int start = 0;
  int end = 0;

  if (rank < extra_rows) {
    start = rank * (base_rows + 1);
    end = start + base_rows;
  } else {
    start = (extra_rows * (base_rows + 1)) + ((rank - extra_rows) * base_rows);
    end = start + base_rows - 1;
  }

  if (start >= count_rows) {
    return {-1, -1};
  }

  if (end >= count_rows) {
    end = count_rows - 1;
  }

  return {start, end};
}

bool RemizovKMaxInMatrixStringMPI::RunImpl() {
  if (GetInput().empty()) {
    return true;
  }

  int size = 0;
  int rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int count_rows = static_cast<int>(GetInput().size());

  std::vector<int> local_interval(2);

  if (rank == 0) {
    std::vector<int> all_intervals(static_cast<std::size_t>(size) * 2);
    for (int i = 0; i < size; ++i) {
      std::vector<int> interval = CalculatingInterval(size, i, count_rows);
      const std::size_t idx1 = static_cast<std::size_t>(2) * static_cast<std::size_t>(i);
      const std::size_t idx2 = idx1 + 1;
      all_intervals[idx1] = interval[0];
      all_intervals[idx2] = interval[1];
    }

    MPI_Scatter(all_intervals.data(), 2, MPI_INT, local_interval.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);
  } else {
    MPI_Scatter(nullptr, 2, MPI_INT, local_interval.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);
  }

  std::vector<int> local_max_values;
  if (local_interval[0] != -1 && local_interval[1] != -1) {
    local_max_values = FindMaxValues(local_interval[0], local_interval[1]);
  }

  const int local_size = static_cast<int>(local_max_values.size());

  std::vector<int> all_sizes(static_cast<std::size_t>(size));
  MPI_Gather(&local_size, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    std::vector<int> displacements(static_cast<std::size_t>(size), 0);
    int total_size = 0;

    for (int i = 0; i < size; ++i) {
      displacements[static_cast<std::size_t>(i)] = total_size;
      total_size += all_sizes[static_cast<std::size_t>(i)];
    }

    GetOutput().resize(static_cast<std::size_t>(total_size));

    MPI_Gatherv(local_max_values.data(), local_size, MPI_INT, GetOutput().data(), all_sizes.data(),
                displacements.data(), MPI_INT, 0, MPI_COMM_WORLD);
  } else {
    MPI_Gatherv(local_max_values.data(), local_size, MPI_INT, nullptr, nullptr, nullptr, MPI_INT, 0, MPI_COMM_WORLD);
  }

  BroadcastResults(rank);

  return true;
}

void RemizovKMaxInMatrixStringMPI::BroadcastResults(int rank) {
  int total_size = 0;

  if (rank == 0) {
    total_size = static_cast<int>(GetOutput().size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    GetOutput().resize(static_cast<std::size_t>(total_size));
  }

  if (total_size > 0) {
    MPI_Bcast(GetOutput().data(), total_size, MPI_INT, 0, MPI_COMM_WORLD);
  }
}

bool RemizovKMaxInMatrixStringMPI::PostProcessingImpl() {
  return true;
}

}  // namespace remizov_k_max_in_matrix_string
