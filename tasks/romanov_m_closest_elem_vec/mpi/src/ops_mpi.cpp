#include "romanov_m_closest_elem_vec/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <cstddef>
#include <limits>
#include <tuple>
#include <vector>

#include "romanov_m_closest_elem_vec/common/include/common.hpp"

namespace romanov_m_closest_elem_vec {

void PerformBoundaryCheck(int rank, int comm_size, int local_sz, int global_offset, const std::vector<int> &local_data,
                          Result &local_res) {
  if (comm_size > 1) {
    int send_val = 0;
    int prev_last_val = 0;
    const int dest = (rank == comm_size - 1) ? MPI_PROC_NULL : rank + 1;
    const int source = (rank == 0) ? MPI_PROC_NULL : rank - 1;

    if (local_sz > 0 && rank != comm_size - 1) {
      send_val = local_data.back();
    }

    MPI_Sendrecv(&send_val, 1, MPI_INT, dest, 0, &prev_last_val, 1, MPI_INT, source, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

    if (rank > 0 && local_sz > 0) {
      const int boundary_idx = global_offset - 1;
      const int boundary_diff = std::abs(local_data[0] - prev_last_val);
      UpdateResult(local_res, boundary_diff, boundary_idx);
    }
  }
}

void CalculateDistribution(int total_size, int comm_size, std::vector<int> &send_counts, std::vector<int> &displs) {
  int base_count = total_size / comm_size;
  int remainder = total_size % comm_size;
  int current_displ = 0;

  send_counts.resize(comm_size);
  displs.resize(comm_size);

  for (int i = 0; i < comm_size; ++i) {
    send_counts[i] = base_count + (i < remainder ? 1 : 0);
    displs[i] = current_displ;
    current_displ += send_counts[i];
  }
}

void UpdateResult(Result &current_res, int new_diff, int new_idx) {
  if (new_diff < current_res.diff) {
    current_res.diff = new_diff;
    current_res.idx = new_idx;
  } else if (new_diff == current_res.diff) {
    if (current_res.idx == -1 || new_idx < current_res.idx) {
      current_res.idx = new_idx;
    }
  }
}

void LocalFindMinDiff(const std::vector<int> &local_data, int local_sz, int global_offset, Result &local_res) {
  if (local_sz >= 2) {
    for (int i = 0; i < local_sz - 1; ++i) {
      const int current_idx = global_offset + i;
      const int diff = std::abs(local_data[i + 1] - local_data[i]);

      UpdateResult(local_res, diff, current_idx);
    }
  }
}

RomanovMClosestElemVecMPI::RomanovMClosestElemVecMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::make_tuple(-1, -1);
}

bool RomanovMClosestElemVecMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int size = 0;
  if (rank == 0) {
    size = static_cast<int>(GetInput().size());
  }

  MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  return size >= 2;
}

bool RomanovMClosestElemVecMPI::PreProcessingImpl() {
  return true;
}

bool RomanovMClosestElemVecMPI::RunImpl() {
  int rank = 0;
  int comm_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

  const size_t global_size = GetInput().size();

  if (global_size < 2) {
    return true;
  }

  std::vector<int> send_counts;
  std::vector<int> displs;

  CalculateDistribution(static_cast<int>(global_size), comm_size, send_counts, displs);

  const int global_offset = displs[rank];
  const int local_sz = send_counts[rank];

  std::vector<int> local_data(local_sz);

  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, send_counts.data(), displs.data(), MPI_INT, local_data.data(),
               local_sz, MPI_INT, 0, MPI_COMM_WORLD);

  Result local_res = {0, 0};
  local_res.diff = std::numeric_limits<int>::max();
  local_res.idx = -1;

  LocalFindMinDiff(local_data, local_sz, global_offset, local_res);

  PerformBoundaryCheck(rank, comm_size, local_sz, global_offset, local_data, local_res);

  Result global_res = {0, 0};
  MPI_Allreduce(&local_res, &global_res, 1, MPI_2INT, MPI_MINLOC, MPI_COMM_WORLD);
  GetOutput() = std::make_tuple(global_res.idx, global_res.idx + 1);
  return true;
}

bool RomanovMClosestElemVecMPI::PostProcessingImpl() {
  return true;
}

}  // namespace romanov_m_closest_elem_vec
