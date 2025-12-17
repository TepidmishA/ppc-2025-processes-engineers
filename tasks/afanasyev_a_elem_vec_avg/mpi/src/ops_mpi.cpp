#include "afanasyev_a_elem_vec_avg/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstdint>
#include <numeric>
#include <vector>

#include "afanasyev_a_elem_vec_avg/common/include/common.hpp"

namespace afanasyev_a_elem_vec_avg {

AfanasyevAElemVecAvgMPI::AfanasyevAElemVecAvgMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool AfanasyevAElemVecAvgMPI::ValidationImpl() {
  return true;
}

bool AfanasyevAElemVecAvgMPI::PreProcessingImpl() {
  return true;
}

bool AfanasyevAElemVecAvgMPI::RunImpl() {
  int rank = 0;
  int num_processes = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

  const InType &global_vec = GetInput();
  int global_n = static_cast<int>(global_vec.size());

  if (global_n == 0) {
    GetOutput() = 0.0;
    MPI_Bcast(static_cast<void *>(&GetOutput()), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    return true;
  }

  std::vector<int> send_counts(num_processes);
  std::vector<int> displs(num_processes);
  int chunk_size = global_n / num_processes;
  int remainder = global_n % num_processes;
  int current_displacement = 0;

  for (int i = 0; i < num_processes; ++i) {
    int count = chunk_size + (i < remainder ? 1 : 0);
    send_counts[i] = count;
    displs[i] = current_displacement;
    current_displacement += count;
  }

  int local_n = send_counts[rank];
  std::vector<int> local_vec(local_n);

  MPI_Scatterv(rank == 0 ? global_vec.data() : nullptr, send_counts.data(), displs.data(), MPI_INT, local_vec.data(),
               local_n, MPI_INT, 0, MPI_COMM_WORLD);

  int64_t local_sum = std::accumulate(local_vec.begin(), local_vec.end(), static_cast<int64_t>(0));
  int64_t global_sum = 0;

  MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT64_T, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    GetOutput() = static_cast<OutType>(global_sum) / static_cast<double>(global_n);
  }

  MPI_Bcast(static_cast<void *>(&GetOutput()), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  return true;
}

bool AfanasyevAElemVecAvgMPI::PostProcessingImpl() {
  return true;
}

}  // namespace afanasyev_a_elem_vec_avg
