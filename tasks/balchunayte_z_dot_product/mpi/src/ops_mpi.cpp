#include "balchunayte_z_dot_product/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <vector>

namespace balchunayte_z_dot_product {

bool BalchunayteZDotProductMPI::ValidationImpl() {
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size_);

  int valid_flag = 0;
  if (world_rank_ == 0) {
    const auto &in = GetInput();
    const auto &a = in.a;
    const auto &b = in.b;

    bool is_valid = true;
    if (a.empty() || b.empty() || a.size() != b.size()) {
      is_valid = false;
    }
    valid_flag = is_valid ? 1 : 0;
  }

  MPI_Bcast(&valid_flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
  return valid_flag == 1;
}

bool BalchunayteZDotProductMPI::PreProcessingImpl() {
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size_);

  int global_size = 0;
  if (world_rank_ == 0) {
    global_size = static_cast<int>(GetInput().a.size());
  }

  MPI_Bcast(&global_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> counts(world_size_, 0);
  std::vector<int> displs(world_size_, 0);

  int base = global_size / world_size_;
  int rem = global_size % world_size_;
  for (int rank_index = 0; rank_index < world_size_; ++rank_index) {
    counts[rank_index] = base + (rank_index < rem ? 1 : 0);
  }
  for (int rank_index = 1; rank_index < world_size_; ++rank_index) {
    displs[rank_index] = displs[rank_index - 1] + counts[rank_index - 1];
  }

  local_size_ = counts[world_rank_];
  local_a_.resize(local_size_);
  local_b_.resize(local_size_);

  double *send_a = nullptr;
  double *send_b = nullptr;
  if (world_rank_ == 0) {
    send_a = GetInput().a.data();
    send_b = GetInput().b.data();
  }

  MPI_Scatterv(send_a, counts.data(), displs.data(), MPI_DOUBLE, local_a_.data(), local_size_, MPI_DOUBLE, 0,
               MPI_COMM_WORLD);

  MPI_Scatterv(send_b, counts.data(), displs.data(), MPI_DOUBLE, local_b_.data(), local_size_, MPI_DOUBLE, 0,
               MPI_COMM_WORLD);

  GetOutput() = 0.0;
  return true;
}

bool BalchunayteZDotProductMPI::RunImpl() {
  double local_sum = 0.0;
  for (int i = 0; i < local_size_; ++i) {
    local_sum += local_a_[i] * local_b_[i];
  }

  double global_sum = 0.0;
  MPI_Allreduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  GetOutput() = global_sum;
  return true;
}

bool BalchunayteZDotProductMPI::PostProcessingImpl() {
  return true;
}

}  // namespace balchunayte_z_dot_product
