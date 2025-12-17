#include "bortsova_a_max_elem_vector/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

#include "bortsova_a_max_elem_vector/common/include/common.hpp"

namespace bortsova_a_max_elem_vector {

BortsovaAMaxElemVectorMpi::BortsovaAMaxElemVectorMpi(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::numeric_limits<int>::min();
}

bool BortsovaAMaxElemVectorMpi::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int is_valid = 0;
  if (rank == 0) {
    is_valid = !GetInput().data.empty() ? 1 : 0;
  }

  MPI_Bcast(&is_valid, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return is_valid == 1;
}

bool BortsovaAMaxElemVectorMpi::PreProcessingImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  GetOutput() = std::numeric_limits<int>::min();

  int vec_size = 0;
  if (rank == 0) {
    vec_size = static_cast<int>(GetInput().data.size());
  }

  MPI_Bcast(&vec_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return vec_size != 0;
}

bool BortsovaAMaxElemVectorMpi::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::vector<int> vec;
  int vec_size = 0;

  if (rank == 0) {
    vec = GetInput().data;
    vec_size = static_cast<int>(vec.size());
  }

  MPI_Bcast(&vec_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (vec_size == 0) {
    return false;
  }

  int chunk_size = vec_size / size;
  int remainder = vec_size % size;

  std::vector<int> sendcounts(size);
  std::vector<int> displs(size);

  int offset = 0;
  for (int i = 0; i < size; i++) {
    sendcounts[i] = chunk_size + (i < remainder ? 1 : 0);
    displs[i] = offset;
    offset += sendcounts[i];
  }

  std::vector<int> local_data(sendcounts[rank]);

  if (rank == 0) {
    MPI_Scatterv(vec.data(), sendcounts.data(), displs.data(), MPI_INT, local_data.data(), sendcounts[rank], MPI_INT, 0,
                 MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(nullptr, sendcounts.data(), displs.data(), MPI_INT, local_data.data(), sendcounts[rank], MPI_INT, 0,
                 MPI_COMM_WORLD);
  }

  int local_max = std::numeric_limits<int>::min();
  if (!local_data.empty()) {
    local_max = local_data[0];
    for (size_t i = 1; i < local_data.size(); i++) {
      local_max = std::max(local_data[i], local_max);
    }
  }

  int global_max = std::numeric_limits<int>::min();
  MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    GetOutput() = global_max;
  }

  return true;
}

bool BortsovaAMaxElemVectorMpi::PostProcessingImpl() {
  return true;
}

}  // namespace bortsova_a_max_elem_vector
