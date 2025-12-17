#include "zhurin_i_matrix_sums/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstdint>
#include <vector>

#include "zhurin_i_matrix_sums/common/include/common.hpp"

namespace zhurin_i_matrix_sums {

ZhurinIMatrixSumsMPI::ZhurinIMatrixSumsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool ZhurinIMatrixSumsMPI::ValidationImpl() {
  return (static_cast<uint64_t>(std::get<0>(GetInput())) * std::get<1>(GetInput()) == std::get<2>(GetInput()).size()) &&
         (GetOutput() == 0.0);
}

bool ZhurinIMatrixSumsMPI::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

bool ZhurinIMatrixSumsMPI::RunImpl() {
  int rank = 0;
  int wsize = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &wsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int rows = static_cast<int>(std::get<0>(GetInput()));
  int columns = static_cast<int>(std::get<1>(GetInput()));

  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&columns, 1, MPI_INT, 0, MPI_COMM_WORLD);
  int size = rows * columns;

  int elems_per_proc = size / wsize;
  int remainder = size % wsize;

  std::vector<int> counts(wsize, 0);
  std::vector<int> displs(wsize, 0);

  int displ = 0;
  for (int proc = 0; proc < wsize; proc++) {
    counts[proc] = elems_per_proc + (proc < remainder ? 1 : 0);
    displs[proc] = displ;
    displ += counts[proc];
  }

  std::vector<double> matrix;
  if (rank == 0) {
    matrix = std::get<2>(GetInput());
  }

  std::vector<double> local_buff(counts[rank], 0);

  MPI_Scatterv(rank == 0 ? matrix.data() : nullptr, counts.data(), displs.data(), MPI_DOUBLE, local_buff.data(),
               counts[rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);

  double local_sum = 0.0;
  for (int i = 0; i < counts[rank]; i++) {
    local_sum += local_buff[i];
  }

  double global_sum = 0.0;
  MPI_Allreduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = global_sum;
  return true;
}

bool ZhurinIMatrixSumsMPI::PostProcessingImpl() {
  return true;
}

}  // namespace zhurin_i_matrix_sums
