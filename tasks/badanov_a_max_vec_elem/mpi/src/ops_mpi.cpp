#include "badanov_a_max_vec_elem/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <climits>
#include <vector>

#include "badanov_a_max_vec_elem/common/include/common.hpp"

namespace badanov_a_max_vec_elem {

BadanovAMaxVecElemMPI::BadanovAMaxVecElemMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    GetInput() = in;
  } else {
    GetInput() = InType();
  }

  GetOutput() = 0;
}

bool BadanovAMaxVecElemMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    return true;
  }

  return true;
}

bool BadanovAMaxVecElemMPI::PreProcessingImpl() {
  return true;
}

bool BadanovAMaxVecElemMPI::RunImpl() {
  int rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int total_elem = 0;
  if (rank == 0) {
    total_elem = static_cast<int>(GetInput().size());
  }

  MPI_Bcast(&total_elem, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (total_elem == 0) {
    GetOutput() = INT_MIN;
    return true;
  }

  int base_size = total_elem / world_size;
  int remainder = total_elem % world_size;

  std::vector<int> local_sizes(world_size);
  std::vector<int> displacements(world_size);

  if (rank == 0) {
    int offset = 0;
    for (int i = 0; i < world_size; ++i) {
      local_sizes[i] = base_size + (i < remainder ? 1 : 0);
      displacements[i] = offset;
      offset += local_sizes[i];
    }
  }

  int local_size = base_size + (rank < remainder ? 1 : 0);
  std::vector<int> local_data(local_size);

  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, local_sizes.data(), displacements.data(), MPI_INT,
               local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

  int max_elem_local = INT_MIN;
  for (int i = 0; i < local_size; ++i) {
    max_elem_local = std::max(local_data[i], max_elem_local);
  }

  int max_elem_global = INT_MIN;
  MPI_Allreduce(&max_elem_local, &max_elem_global, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  GetOutput() = max_elem_global;
  return true;
}

bool BadanovAMaxVecElemMPI::PostProcessingImpl() {
  return true;
}

}  // namespace badanov_a_max_vec_elem
