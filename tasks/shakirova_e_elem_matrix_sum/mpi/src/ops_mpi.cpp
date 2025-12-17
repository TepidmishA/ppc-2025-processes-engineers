#include "shakirova_e_elem_matrix_sum/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "shakirova_e_elem_matrix_sum/common/include/common.hpp"
#include "shakirova_e_elem_matrix_sum/common/include/matrix.hpp"

namespace shakirova_e_elem_matrix_sum {

ShakirovaEElemMatrixSumMPI::ShakirovaEElemMatrixSumMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool ShakirovaEElemMatrixSumMPI::ValidationImpl() {
  int rank = -1;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    return GetInput().IsValid();
  }

  return true;
}

bool ShakirovaEElemMatrixSumMPI::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

bool ShakirovaEElemMatrixSumMPI::RunImpl() {
  int rank = 0;
  int p_count = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &p_count);

  if (p_count <= 0) {
    return false;
  }

  size_t row_count = GetInput().rows;
  size_t col_count = GetInput().cols;

  MPI_Bcast(&row_count, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(&col_count, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);

  size_t rows_per_process = row_count / p_count;
  size_t remaining_rows = row_count % p_count;

  size_t my_rows = rows_per_process;
  if (std::cmp_less(rank, remaining_rows)) {
    my_rows = my_rows + 1;
  }

  size_t elements_count = my_rows * col_count;
  std::vector<int64_t> my_data(elements_count);

  if (rank == 0) {
    std::vector<int> send_counts(p_count);
    std::vector<int> displacements(p_count, 0);

    for (int i = 0; i < p_count; i++) {
      size_t proc_rows = rows_per_process;
      if (std::cmp_less(i, remaining_rows)) {
        proc_rows = proc_rows + 1;
      }
      send_counts[i] = static_cast<int>(proc_rows * col_count);

      if (i > 0) {
        displacements[i] = displacements[i - 1] + send_counts[i - 1];
      }
    }

    MPI_Scatterv(GetInput().data.data(), send_counts.data(), displacements.data(), MPI_INT64_T, my_data.data(),
                 static_cast<int>(elements_count), MPI_INT64_T, 0, MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(nullptr, nullptr, nullptr, MPI_INT64_T, my_data.data(), static_cast<int>(elements_count), MPI_INT64_T,
                 0, MPI_COMM_WORLD);
  }

  int64_t local_sum = 0;
  for (const auto &value : my_data) {
    local_sum = local_sum + value;
  }

  int64_t total_sum = 0;
  MPI_Allreduce(&local_sum, &total_sum, 1, MPI_INT64_T, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = total_sum;

  return true;
}

bool ShakirovaEElemMatrixSumMPI::PostProcessingImpl() {
  return true;
}

}  // namespace shakirova_e_elem_matrix_sum
