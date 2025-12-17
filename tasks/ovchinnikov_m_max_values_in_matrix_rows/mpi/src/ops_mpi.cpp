#include "ovchinnikov_m_max_values_in_matrix_rows/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

#include "ovchinnikov_m_max_values_in_matrix_rows/common/include/common.hpp"

namespace ovchinnikov_m_max_values_in_matrix_rows {

OvchinnikovMMaxValuesInMatrixRowsMPI::OvchinnikovMMaxValuesInMatrixRowsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  static_cast<void>(GetOutput());
}

bool OvchinnikovMMaxValuesInMatrixRowsMPI::ValidationImpl() {
  return true;
}

bool OvchinnikovMMaxValuesInMatrixRowsMPI::PreProcessingImpl() {
  return true;
}

bool OvchinnikovMMaxValuesInMatrixRowsMPI::RunImpl() {
  int tmp_rank = 0;
  int tmp_proc_amount = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &tmp_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &tmp_proc_amount);
  auto rank = static_cast<size_t>(tmp_rank);
  auto proc_amount = static_cast<size_t>(tmp_proc_amount);

  const size_t lines = std::get<0>(GetInput());
  const size_t cols = std::get<1>(GetInput());
  if (lines == 0 || cols == 0) {
    return true;
  }
  const auto &matrix = std::get<2>(GetInput());

  std::vector<int> elem_count(proc_amount);
  std::vector<int> elem_offset(proc_amount);
  if (rank == 0) {
    const size_t chunk_base = lines / proc_amount;
    const size_t chunk_extra = lines % proc_amount;
    for (size_t process = 0; process < proc_amount; process++) {
      const size_t line_begin = (process * chunk_base) + std::min(process, chunk_extra);
      size_t line_end = line_begin + chunk_base;
      if (process < chunk_extra) {
        line_end++;
      }

      size_t elems = (line_end - line_begin) * cols;
      size_t offset = line_begin * cols;
      elem_count[process] = static_cast<int>(elems);
      elem_offset[process] = static_cast<int>(offset);
    }
  }

  MPI_Bcast(elem_count.data(), static_cast<int>(proc_amount), MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(elem_offset.data(), static_cast<int>(proc_amount), MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> local_data(elem_count[rank]);

  const int *matrix_buffer = nullptr;
  if (rank == 0) {
    matrix_buffer = matrix.data();
  }
  MPI_Scatterv(matrix_buffer,  // only for rank 0
               elem_count.data(), elem_offset.data(), MPI_INT, local_data.data(), elem_count[rank], MPI_INT, 0,
               MPI_COMM_WORLD);

  std::vector<int> local_max(cols, std::numeric_limits<int>::min());

  size_t local_lines = local_data.size() / cols;

  for (size_t i = 0; i < local_lines; i++) {
    const size_t local_base = i * cols;
    for (size_t j = 0; j < cols; j++) {
      local_max[j] = std::max(local_max[j], local_data[local_base + j]);
    }
  }
  std::vector<int> global_max(cols);
  MPI_Allreduce(local_max.data(), global_max.data(), static_cast<int>(cols), MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  GetOutput() = global_max;
  return true;
}

bool OvchinnikovMMaxValuesInMatrixRowsMPI::PostProcessingImpl() {
  return true;
}

}  // namespace ovchinnikov_m_max_values_in_matrix_rows
