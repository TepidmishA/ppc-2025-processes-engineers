#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <functional>
#include <numeric>
#include <vector>

#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/common/include/common.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PerepelkinIMatrixMultHorizontalStripOnlyAMPI(const InType &in) {
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num_);

  SetTypeOfTask(GetStaticTypeOfTask());
  if (proc_rank_ == 0) {
    GetInput() = in;
  }
  GetOutput() = std::vector<std::vector<double>>();
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::ValidationImpl() {
  bool is_valid = true;
  if (proc_rank_ == 0) {
    is_valid = RootValidationImpl();
  }

  MPI_Bcast(&is_valid, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
  return (is_valid && GetOutput().empty());
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::RootValidationImpl() {
  const auto &[matrix_a, matrix_b] = GetInput();

  if (matrix_a.empty() || matrix_b.empty()) {
    return false;
  }

  const size_t width_a = matrix_a[0].size();
  const size_t width_b = matrix_b[0].size();
  const size_t height_b = matrix_b.size();

  if (width_a != height_b) {
    return false;
  }

  if (!CheckConsistentRowWidths(matrix_a, width_a)) {
    return false;
  }

  if (!CheckConsistentRowWidths(matrix_b, width_b)) {
    return false;
  }

  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::CheckConsistentRowWidths(
    const std::vector<std::vector<double>> &matrix, size_t expected_width) {
  for (size_t i = 1; i < matrix.size(); ++i) {
    if (matrix[i].size() != expected_width) {
      return false;
    }
  }
  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PreProcessingImpl() {
  if (proc_rank_ == 0) {
    const auto &[matrix_a, matrix_b] = GetInput();

    // Store original sizes on root
    height_a_ = matrix_a.size();
    height_b_ = matrix_b.size();
    width_a_ = matrix_a[0].size();
    width_b_ = matrix_b[0].size();

    // Flatten matrix A
    flat_a_.reserve(width_a_ * height_a_);
    for (const auto &row : matrix_a) {
      flat_a_.insert(flat_a_.end(), row.begin(), row.end());
    }

    // Create transposed-and-flattened matrix B
    flat_b_t_.resize(width_b_ * width_a_);
    for (size_t row = 0; row < width_a_; row++) {
      for (size_t col = 0; col < width_b_; col++) {
        flat_b_t_[(col * width_a_) + row] = matrix_b[row][col];
      }
    }
  }

  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::RunImpl() {
  // [1] Broadcast matrix sizes and matrix B
  BcastMatrixSizes();
  BcastMatrixB();

  // [2] Distribute matrix A
  std::vector<double> local_a;
  std::vector<int> rows_per_rank;
  const size_t local_rows = DistributeMatrixA(local_a, rows_per_rank);

  // [3] Local computation of matrix C
  std::vector<double> local_c(local_rows * width_b_);
  for (size_t row_a = 0; row_a < local_rows; row_a++) {
    const auto a_it = local_a.begin() + static_cast<DiffT>(row_a * width_a_);
    const auto a_end = a_it + static_cast<DiffT>(width_a_);

    for (size_t col_b = 0; col_b < width_b_; col_b++) {
      const auto b_it = flat_b_t_.begin() + static_cast<DiffT>(col_b * width_a_);
      local_c[(row_a * width_b_) + col_b] =
          std::transform_reduce(a_it, a_end, b_it, 0.0, std::plus<>(), std::multiplies<>());
    }
  }

  // [4] Gather local results
  GatherAndBcastResult(rows_per_rank, local_c);
  return true;
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::BcastMatrixSizes() {
  MPI_Bcast(&height_a_, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&height_b_, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&width_a_, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&width_b_, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::BcastMatrixB() {
  const size_t total_t = width_b_ * height_b_;

  if (proc_rank_ != 0) {
    flat_b_t_.resize(total_t);
  }

  MPI_Bcast(flat_b_t_.data(), static_cast<int>(total_t), MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

int PerepelkinIMatrixMultHorizontalStripOnlyAMPI::DistributeMatrixA(std::vector<double> &local_a,
                                                                    std::vector<int> &rows_per_rank) {
  // Determine rows per rank
  rows_per_rank.resize(proc_num_);
  const int base_rows = static_cast<int>(height_a_ / proc_num_);
  const int remainder_rows = static_cast<int>(height_a_ % proc_num_);
  for (int i = 0; i < proc_num_; i++) {
    rows_per_rank[i] = base_rows + (i < remainder_rows ? 1 : 0);
  }

  // Prepare counts and displacements
  std::vector<int> counts(proc_num_);
  std::vector<int> displacements(proc_num_);
  for (int i = 0, offset = 0; i < proc_num_; i++) {
    counts[i] = rows_per_rank[i] * static_cast<int>(width_a_);
    displacements[i] = offset;
    offset += counts[i];
  }

  const int local_a_size = rows_per_rank[proc_rank_] * static_cast<int>(width_a_);
  local_a.resize(local_a_size);

  MPI_Scatterv(flat_a_.data(), counts.data(), displacements.data(), MPI_DOUBLE, local_a.data(), local_a_size,
               MPI_DOUBLE, 0, MPI_COMM_WORLD);

  return rows_per_rank[proc_rank_];
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::GatherAndBcastResult(const std::vector<int> &rows_per_rank,
                                                                        const std::vector<double> &local_c) {
  std::vector<int> counts(proc_num_);
  std::vector<int> displacements(proc_num_);
  for (int i = 0, offset = 0; i < proc_num_; i++) {
    counts[i] = rows_per_rank[i] * static_cast<int>(width_b_);
    displacements[i] = offset;
    offset += counts[i];
  }

  flat_c_.resize(height_a_ * width_b_);

  MPI_Allgatherv(local_c.data(), counts[proc_rank_], MPI_DOUBLE, flat_c_.data(), counts.data(), displacements.data(),
                 MPI_DOUBLE, MPI_COMM_WORLD);
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PostProcessingImpl() {
  auto &output = GetOutput();
  output.resize(height_a_);
  for (auto &row : output) {
    row.resize(width_b_);
  }

  for (size_t i = 0; i < height_a_; i++) {
    for (size_t j = 0; j < width_b_; j++) {
      output[i][j] = flat_c_[(i * width_b_) + j];
    }
  }

  return true;
}

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
