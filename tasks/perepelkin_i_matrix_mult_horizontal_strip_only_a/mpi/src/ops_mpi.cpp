#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
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
    const auto &[matrix_a, matrix_b] = GetInput();

    if (matrix_a.empty() || matrix_b.empty()) {
      is_valid = false;
    }
    else {
      const size_t width_a = matrix_a[0].size();
      const size_t width_b = matrix_b[0].size();
      const size_t height_b = matrix_b.size();

      if (width_a != height_b) {
        is_valid = false;
      }

      for (size_t i = 1; i < matrix_a.size(); i++) {
        if (matrix_a[i].size() != width_a) {
          is_valid = false;
          break;
        }
      }

      for (size_t i = 1; i < matrix_b.size(); i++) {
        if (matrix_b[i].size() != width_b) {
          is_valid = false;
          break;
        }
      }
    }
  }

  MPI_Bcast(&is_valid, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
  return (is_valid && (GetOutput() == std::vector<std::vector<double>>()));
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PreProcessingImpl() {
  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::RunImpl() {
  // [1] Broadcast matrix sizes
  size_t height_a = 0;
  size_t height_b = 0;
  size_t width_a = 0;
  size_t width_b = 0;

  BcastMatrixSizes(height_a, width_a, height_b, width_b);

  // [2] Broadcast matrix B
  std::vector<double> flat_b;
  BcastMatrixB(height_b, width_b, flat_b);

  // [3] Distribute matrix A
  std::vector<double> local_a;
  std::vector<int> rows_per_rank;
  const size_t local_rows = DistributeMatrixA(height_a, width_a, local_a, rows_per_rank);

  // [4] Local computation of matrix C
  std::vector<double> local_c(local_rows * width_b);
  for (size_t row_a = 0; row_a < local_rows; row_a++) {
    const size_t row_offset = row_a * width_a;

    for (size_t col_b = 0; col_b < width_b; col_b++) {
      double tmp_sum = 0.0;

      for (size_t i = 0; i < width_a; i++) {
        tmp_sum += local_a[row_offset + i] * flat_b[i * width_b + col_b];
      }
      local_c[row_a * width_b + col_b] = tmp_sum;
    }
  }

  // [5] Gather local results
  std::vector<double> flat_c;
  GatherAndBcastResult(height_a, width_b, rows_per_rank, local_c, flat_c);

  // [6] Convert flat matrix C to output format
  PrepareOutput(height_a, width_b, flat_c);
  return true;
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::BcastMatrixSizes(size_t &height_a, size_t &width_a, size_t &height_b,
                                                                    size_t &width_b) {
  if (proc_rank_ == 0) {
    const auto &[matrix_a, matrix_b] = GetInput();
    height_a = matrix_a.size();
    height_b = matrix_b.size();
    width_a = matrix_a[0].size();
    width_b = matrix_b[0].size();
  }

  MPI_Bcast(&height_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&height_b, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&width_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&width_b, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::BcastMatrixB(const size_t &height_b, const size_t &width_b,
                                                                std::vector<double> &flat_b) {
  const size_t total_b = height_b * width_b;

  if (proc_rank_ == 0) {
    const auto &[matrix_a, matrix_b] = GetInput();
    for (const auto &row : matrix_b) {
      flat_b.insert(flat_b.end(), row.begin(), row.end());
    }
  } else {
    flat_b.reserve(total_b);
  }

  MPI_Bcast(flat_b.data(), static_cast<int>(total_b), MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

int PerepelkinIMatrixMultHorizontalStripOnlyAMPI::DistributeMatrixA(const size_t &height_a, const size_t &width_a,
                                                                    std::vector<double> &local_a,
                                                                    std::vector<int> &rows_per_rank) {
  std::vector<double> flat_a;
  if (proc_rank_ == 0) {
    const auto &[matrix_a, matrix_b] = GetInput();
    for (const auto &row : matrix_a) {
      flat_a.insert(flat_a.end(), row.begin(), row.end());
    }
  }

  // Determine rows per rank
  rows_per_rank.reserve(proc_num_);
  const int base_rows = static_cast<int>(height_a / proc_num_);
  const int remainder_rows = static_cast<int>(height_a % proc_num_);
  for (int i = 0; i < proc_num_; i++) {
    rows_per_rank[i] = base_rows + (i < remainder_rows ? 1 : 0);
  }

  // Prepare counts and displacements
  std::vector<int> counts(proc_num_);
  std::vector<int> displacements(proc_num_);
  if (proc_rank_ == 0) {
    for (int i = 0, offset = 0; i < proc_num_; i++) {
      counts[i] = rows_per_rank[i] * static_cast<int>(width_a);
      displacements[i] = offset;
      offset += counts[i];
    }
  }

  const int local_a_size = rows_per_rank[proc_rank_] * static_cast<int>(width_a);
  local_a.resize(local_a_size);

  MPI_Scatterv(flat_a.data(), counts.data(), displacements.data(), MPI_DOUBLE, local_a.data(), local_a_size, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

  return rows_per_rank[proc_rank_];
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::GatherAndBcastResult(const size_t &height_a, const size_t &width_b,
                                                                        const std::vector<int> &rows_per_rank,
                                                                        const std::vector<double> &local_c,
                                                                        std::vector<double> &flat_c) {
  std::vector<int> counts(proc_num_);
  std::vector<int> displacements(proc_num_);
  for (int i = 0, offset = 0; i < proc_num_; i++) {
    counts[i] = rows_per_rank[i] * width_b;
    displacements[i] = offset;
    offset += counts[i];
  }

  flat_c.reserve(height_a * width_b);
  MPI_Allgatherv(local_c.data(), counts[proc_rank_], MPI_DOUBLE, flat_c.data(), counts.data(), displacements.data(),
                 MPI_DOUBLE, MPI_COMM_WORLD);
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PrepareOutput(const size_t &height_a, const size_t &width_b,
                                                                 const std::vector<double> &flat_c) {
  auto &output = GetOutput();
  output.assign(height_a, std::vector<double>(width_b));

  for (size_t i = 0; i < height_a; i++) {
    for (size_t j = 0; j < width_b; j++) {
      output[i][j] = flat_c[i * width_b + j];
    }
  }
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
