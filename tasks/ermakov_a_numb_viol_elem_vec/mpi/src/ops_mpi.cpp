#include "ermakov_a_numb_viol_elem_vec/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <vector>

#include "ermakov_a_numb_viol_elem_vec/common/include/common.hpp"

namespace ermakov_a_numb_viol_elem_vec {

namespace {

void ComputeBlocks(int total_size, int world_size, std::vector<int> &counts, std::vector<int> &displs) {
  counts.resize(world_size);
  displs.resize(world_size);

  const int base = total_size / world_size;
  const int rem = total_size % world_size;

  int shift = 0;
  for (int rank = 0; rank < world_size; ++rank) {
    counts[rank] = base + (rank < rem ? 1 : 0);  // ? : чтобы прошел clang-tidy
    displs[rank] = shift;
    shift += counts[rank];
  }
}

void ScatterData(const std::vector<int> &input, std::vector<int> &local, const std::vector<int> &counts,
                 const std::vector<int> &displs, int rank) {
  int world_size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (counts[rank] == 0) {
    return;
  }

  if (rank == 0) {
    for (int dest = 1; dest < world_size; ++dest) {
      if (counts[dest] > 0) {
        MPI_Send(input.data() + displs[dest], counts[dest], MPI_INT, dest, 0, MPI_COMM_WORLD);
      }
    }
    local.assign(input.begin(), input.begin() + counts[0]);
  } else {
    MPI_Recv(local.data(), counts[rank], MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

int CountLocalViolations(const std::vector<int> &local) {
  int count = 0;
  for (std::size_t i = 0; i + 1 < local.size(); ++i) {
    if (local[i] > local[i + 1]) {
      ++count;
    }
  }
  return count;
}

int CheckBoundaryViolation(const std::vector<int> &local, const std::vector<int> &counts, int rank, int world_size) {
  if (local.empty()) {
    return 0;
  }

  int left_last_value = 0;
  const int my_last_value = local.back();

  if (rank > 0) {
    MPI_Recv(&left_last_value, 1, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  if (rank + 1 < world_size && counts[rank + 1] > 0) {
    MPI_Send(&my_last_value, 1, MPI_INT, rank + 1, 1, MPI_COMM_WORLD);
  }

  if (rank > 0 && left_last_value > local.front()) {
    return 1;
  }

  return 0;
}

}  // namespace

ErmakovANumbViolElemVecMPI::ErmakovANumbViolElemVecMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool ErmakovANumbViolElemVecMPI::ValidationImpl() {
  return true;
}
bool ErmakovANumbViolElemVecMPI::PreProcessingImpl() {
  return true;
}

bool ErmakovANumbViolElemVecMPI::RunImpl() {
  int rank = 0;
  int world_size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  const auto &input = GetInput();
  int n = static_cast<int>(input.size());

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n == 0) {
    GetOutput() = 0;
    return true;
  }

  std::vector<int> counts;
  std::vector<int> displs;
  ComputeBlocks(n, world_size, counts, displs);

  std::vector<int> local(counts[rank]);

  ScatterData(input, local, counts, displs, rank);

  const int local_viol = CountLocalViolations(local);
  const int border_viol = CheckBoundaryViolation(local, counts, rank, world_size);

  const int local_sum = local_viol + border_viol;

  int res = 0;
  MPI_Reduce(&local_sum, &res, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Bcast(&res, 1, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput() = res;
  return true;
}

bool ErmakovANumbViolElemVecMPI::PostProcessingImpl() {
  return true;
}

}  // namespace ermakov_a_numb_viol_elem_vec
