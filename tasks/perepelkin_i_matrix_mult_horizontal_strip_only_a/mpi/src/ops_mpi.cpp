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
  if (proc_rank_ == 0) {
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

    for (size_t i = 1; i < matrix_a.size(); i++) {
      if (matrix_a[i].size() != width_a) {
        return false;
      }
    }

    for (size_t i = 1; i < matrix_b.size(); i++) {
      if (matrix_b[i].size() != width_b) {
        return false;
      }
    }
  }

  return (GetOutput() == std::vector<std::vector<double>>());
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PreProcessingImpl() {
  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::RunImpl() {
  int height_a = 0;
  int width_a = 0;
  int width_b = 0;

  std::vector<double> flat_a;
  std::vector<double> flat_b;

  PrepareRootData(height_a, width_a, width_b, flat_a, flat_b);
  BroadcastData(height_a, width_a, width_b, flat_b);

  if (height_a == 0 || width_a == 0 || width_b == 0) {
    GetOutput().clear();
    return true;
  }

  std::vector<double> local_a;
  std::vector<int> rows_per_rank;
  const int local_rows = DistributeMatrixA(height_a, width_a, local_a, rows_per_rank, flat_a);
  std::vector<double> local_c(local_rows * width_b, 0.0);

  for (int r = 0; r < local_rows; ++r) {
    const int row_offset = r * width_a;
    for (int c = 0; c < width_b; ++c) {
      double acc = 0.0;
      for (int k = 0; k < width_a; ++k) {
        acc += local_a[row_offset + k] * flat_b[k * width_b + c];
      }
      local_c[r * width_b + c] = acc;
    }
  }

  std::vector<double> flat_c;
  GatherAndBroadcastResult(height_a, width_b, rows_per_rank, local_c, flat_c);

  PopulateOutput(height_a, width_b, flat_c);

  return true;
}

int PerepelkinIMatrixMultHorizontalStripOnlyAMPI::DistributeMatrixA(int height_a, int width_a, std::vector<double> &local_a,
                                                      std::vector<int> &rows_per_rank, const std::vector<double> &flat_a) {
  rows_per_rank.assign(proc_num_, 0);

  const int base_rows = height_a / proc_num_;
  const int remainder_rows = height_a % proc_num_;
  for (int i = 0; i < proc_num_; i++) {
    rows_per_rank[i] = base_rows + (i < remainder_rows ? 1 : 0);
  }

  std::vector<int> send_counts(proc_num_, 0);
  std::vector<int> send_displs(proc_num_, 0);
  int offset = 0;
  for (int i = 0; i < proc_num_; i++) {
    send_counts[i] = rows_per_rank[i] * width_a;
    send_displs[i] = offset;
    offset += send_counts[i];
  }

  const int local_a_count = send_counts[proc_rank_];
  local_a.assign(local_a_count, 0.0);

  MPI_Scatterv(proc_rank_ == 0 ? flat_a.data() : nullptr, send_counts.data(), send_displs.data(), MPI_DOUBLE,
               local_a.data(), local_a_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  return rows_per_rank[proc_rank_];
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PrepareRootData(int &height_a, int &width_a, int &width_b,
                                                        std::vector<double> &flat_a, std::vector<double> &flat_b) {
  if (proc_rank_ != 0) {
    return;
  }

  const auto &[matrix_a, matrix_b] = GetInput();
  height_a = static_cast<int>(matrix_a.size());
  width_a = static_cast<int>(matrix_a[0].size());
  width_b = static_cast<int>(matrix_b[0].size());

  flat_a.reserve(height_a * width_a);
  for (const auto &row : matrix_a) {
    flat_a.insert(flat_a.end(), row.begin(), row.end());
  }

  flat_b.reserve(width_a * width_b);
  for (const auto &row : matrix_b) {
    flat_b.insert(flat_b.end(), row.begin(), row.end());
  }
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::BroadcastData(int &height_a, int &width_a, int &width_b,
                                                                    std::vector<double> &flat_b) {
  // Bcast matrix sizes
  MPI_Bcast(&height_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&width_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&width_b, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  // Bcast matrix B
  const auto total_b = static_cast<std::size_t>(width_a) * width_b;
  flat_b.resize(total_b);
  if (total_b > 0) {
    MPI_Bcast(flat_b.data(), static_cast<int>(total_b), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::GatherAndBroadcastResult(int height_a, int width_b,
                                                                 const std::vector<int> &rows_per_rank,
                                                                 const std::vector<double> &local_c,
                                                                 std::vector<double> &flat_c) {
  std::vector<int> recv_counts(proc_num_, 0);
  std::vector<int> recv_displs(proc_num_, 0);
  int offset = 0;
  for (int i = 0; i < proc_num_; ++i) {
    recv_counts[i] = rows_per_rank[i] * width_b;
    recv_displs[i] = offset;
    offset += recv_counts[i];
  }

  flat_c.assign(height_a * width_b, 0.0);

  const int local_c_count = recv_counts[proc_rank_];
  MPI_Gatherv(local_c.data(), local_c_count, MPI_DOUBLE, proc_rank_ == 0 ? flat_c.data() : nullptr, recv_counts.data(),
              recv_displs.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(flat_c.data(), height_a * width_b, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PopulateOutput(int height_a, int width_b,
                                                       const std::vector<double> &flat_c) {
  auto &output = GetOutput();
  output.assign(height_a, std::vector<double>(width_b, 0.0));

  for (int i = 0; i < height_a; ++i) {
    for (int j = 0; j < width_b; ++j) {
      output[i][j] = flat_c[i * width_b + j];
    }
  }
}

bool PerepelkinIMatrixMultHorizontalStripOnlyAMPI::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
