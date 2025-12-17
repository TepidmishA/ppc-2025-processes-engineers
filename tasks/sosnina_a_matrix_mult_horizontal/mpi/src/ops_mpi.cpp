#include "sosnina_a_matrix_mult_horizontal/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

#include "sosnina_a_matrix_mult_horizontal/common/include/common.hpp"

namespace sosnina_a_matrix_mult_horizontal {

SosninaAMatrixMultHorizontalMPI::SosninaAMatrixMultHorizontalMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetOutput() = std::vector<std::vector<double>>();

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    matrix_A_ = in.first;
    matrix_B_ = in.second;
  }
}

bool SosninaAMatrixMultHorizontalMPI::ValidationImpl() {
  int mpi_initialized = 0;
  MPI_Initialized(&mpi_initialized);

  if (mpi_initialized == 0) {
    return false;
  }

  int size = 1;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  return size >= 1;
}

bool SosninaAMatrixMultHorizontalMPI::PreProcessingImpl() {
  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  rank_ = rank;
  world_size_ = size;
  GetOutput() = std::vector<std::vector<double>>();

  return true;
}

bool SosninaAMatrixMultHorizontalMPI::RunImpl() {
  if (world_size_ == 1) {
    return RunSequential();
  }

  int rows_a = 0;
  int cols_a = 0;
  int rows_b = 0;
  int cols_b = 0;

  if (!PrepareAndValidateSizes(rows_a, cols_a, rows_b, cols_b)) {
    return true;
  }

  std::vector<double> b_flat(static_cast<size_t>(rows_b) * static_cast<size_t>(cols_b));
  PrepareAndBroadcastMatrixB(b_flat, rows_b, cols_b);

  std::vector<int> my_row_indices;
  std::vector<double> local_a_flat;
  int local_rows = 0;
  DistributeMatrixAData(my_row_indices, local_a_flat, local_rows, rows_a, cols_a);

  std::vector<double> local_result_flat(static_cast<size_t>(local_rows) * static_cast<size_t>(cols_b), 0.0);
  ComputeLocalMultiplication(local_a_flat, b_flat, local_result_flat, local_rows, cols_a, cols_b);

  std::vector<double> final_result_flat;
  GatherResults(final_result_flat, my_row_indices, local_result_flat, local_rows, rows_a, cols_b);

  ConvertToMatrix(final_result_flat, rows_a, cols_b);

  return true;
}

bool SosninaAMatrixMultHorizontalMPI::RunSequential() {
  if (rank_ != 0) {
    return true;
  }

  const auto &matrix_a = matrix_A_;
  const auto &matrix_b = matrix_B_;

  if (matrix_a.empty() || matrix_b.empty()) {
    GetOutput() = std::vector<std::vector<double>>();
    return true;
  }

  size_t rows_a = matrix_a.size();
  size_t cols_a = matrix_a[0].size();
  size_t cols_b = matrix_b[0].size();

  auto &output = GetOutput();
  output = std::vector<std::vector<double>>(rows_a, std::vector<double>(cols_b, 0.0));

  for (size_t i = 0; i < rows_a; ++i) {
    for (size_t k = 0; k < cols_a; ++k) {
      double aik = matrix_a[i][k];
      for (size_t j = 0; j < cols_b; ++j) {
        output[i][j] += aik * matrix_b[k][j];
      }
    }
  }

  return true;
}

bool SosninaAMatrixMultHorizontalMPI::PrepareAndValidateSizes(int &rows_a, int &cols_a, int &rows_b, int &cols_b) {
  if (rank_ == 0) {
    rows_a = static_cast<int>(matrix_A_.size());
    cols_a = rows_a > 0 ? static_cast<int>(matrix_A_[0].size()) : 0;
    rows_b = static_cast<int>(matrix_B_.size());
    cols_b = rows_b > 0 ? static_cast<int>(matrix_B_[0].size()) : 0;
  }

  std::array<int, 4> sizes = {rows_a, cols_a, rows_b, cols_b};
  MPI_Bcast(sizes.data(), 4, MPI_INT, 0, MPI_COMM_WORLD);

  rows_a = sizes[0];
  cols_a = sizes[1];
  rows_b = sizes[2];
  cols_b = sizes[3];

  if (cols_a != rows_b || rows_a == 0 || cols_a == 0 || rows_b == 0 || cols_b == 0) {
    GetOutput() = std::vector<std::vector<double>>();
    return false;
  }

  return true;
}

void SosninaAMatrixMultHorizontalMPI::PrepareAndBroadcastMatrixB(std::vector<double> &b_flat, int rows_b, int cols_b) {
  if (rank_ == 0) {
    for (int i = 0; i < rows_b; ++i) {
      for (int j = 0; j < cols_b; ++j) {
        b_flat[(static_cast<size_t>(i) * static_cast<size_t>(cols_b)) + static_cast<size_t>(j)] = matrix_B_[i][j];
      }
    }
  }

  MPI_Bcast(b_flat.data(), rows_b * cols_b, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void SosninaAMatrixMultHorizontalMPI::FillLocalAFlat(const std::vector<int> &my_row_indices,
                                                     std::vector<double> &local_a_flat, int cols_a) {
  for (size_t idx = 0; idx < my_row_indices.size(); ++idx) {
    int global_row = my_row_indices[idx];
    for (int j = 0; j < cols_a; ++j) {
      local_a_flat[(idx * static_cast<size_t>(cols_a)) + static_cast<size_t>(j)] = matrix_A_[global_row][j];
    }
  }
}

void SosninaAMatrixMultHorizontalMPI::SendRowsToProcess(int dest, const std::vector<int> &dest_rows, int cols_a) {
  int dest_row_count = static_cast<int>(dest_rows.size());
  MPI_Send(&dest_row_count, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

  if (dest_row_count > 0) {
    std::vector<int> rows_copy = dest_rows;
    MPI_Send(rows_copy.data(), dest_row_count, MPI_INT, dest, 1, MPI_COMM_WORLD);

    std::vector<double> buffer(static_cast<size_t>(dest_row_count) * static_cast<size_t>(cols_a));
    for (int idx = 0; idx < dest_row_count; ++idx) {
      int global_row = dest_rows[idx];
      for (int j = 0; j < cols_a; ++j) {
        buffer[(static_cast<size_t>(idx) * static_cast<size_t>(cols_a)) + static_cast<size_t>(j)] =
            matrix_A_[global_row][j];
      }
    }

    MPI_Send(buffer.data(), dest_row_count * cols_a, MPI_DOUBLE, dest, 2, MPI_COMM_WORLD);
  }
}

std::vector<int> SosninaAMatrixMultHorizontalMPI::GetRowsForProcess(int process_rank, int rows_a) const {
  std::vector<int> rows;
  for (int i = 0; i < rows_a; ++i) {
    if (i % world_size_ == process_rank) {
      rows.push_back(i);
    }
  }
  return rows;
}

void SosninaAMatrixMultHorizontalMPI::ReceiveRowsFromRoot(int &local_rows, std::vector<int> &my_row_indices,
                                                          std::vector<double> &local_a_flat, int cols_a) {
  MPI_Recv(&local_rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  if (local_rows > 0) {
    my_row_indices.resize(static_cast<size_t>(local_rows));
    local_a_flat.resize(static_cast<size_t>(local_rows) * static_cast<size_t>(cols_a));

    MPI_Recv(my_row_indices.data(), local_rows, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(local_a_flat.data(), local_rows * cols_a, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

void SosninaAMatrixMultHorizontalMPI::DistributeMatrixAData(std::vector<int> &my_row_indices,
                                                            std::vector<double> &local_a_flat, int &local_rows,
                                                            int rows_a, int cols_a) {
  local_rows = (rows_a / world_size_) + (rank_ < (rows_a % world_size_) ? 1 : 0);

  my_row_indices = GetRowsForProcess(rank_, rows_a);

  if (my_row_indices.size() != static_cast<size_t>(local_rows)) {
    local_rows = static_cast<int>(my_row_indices.size());
  }

  local_a_flat.resize(static_cast<size_t>(local_rows) * static_cast<size_t>(cols_a));

  if (rank_ == 0) {
    FillLocalAFlat(my_row_indices, local_a_flat, cols_a);

    for (int dest = 1; dest < world_size_; ++dest) {
      std::vector<int> dest_rows = GetRowsForProcess(dest, rows_a);
      SendRowsToProcess(dest, dest_rows, cols_a);
    }
  } else {
    ReceiveRowsFromRoot(local_rows, my_row_indices, local_a_flat, cols_a);
  }
}

void SosninaAMatrixMultHorizontalMPI::CollectLocalResults(const std::vector<int> &my_row_indices,
                                                          const std::vector<double> &local_result_flat,
                                                          std::vector<double> &final_result_flat, int cols_b) {
  for (size_t idx = 0; idx < my_row_indices.size(); ++idx) {
    int global_row = my_row_indices[idx];
    for (int j = 0; j < cols_b; ++j) {
      final_result_flat[(static_cast<size_t>(global_row) * static_cast<size_t>(cols_b)) + static_cast<size_t>(j)] =
          local_result_flat[(idx * static_cast<size_t>(cols_b)) + static_cast<size_t>(j)];
    }
  }
}

void SosninaAMatrixMultHorizontalMPI::ReceiveResultsFromProcess(int src, std::vector<double> &final_result_flat,
                                                                int cols_b) const {
  int rows_a = static_cast<int>(final_result_flat.size() / cols_b);
  std::vector<int> src_rows = GetRowsForProcess(src, rows_a);
  int src_row_count = static_cast<int>(src_rows.size());

  if (src_row_count > 0) {
    std::vector<double> buffer(static_cast<size_t>(src_row_count) * static_cast<size_t>(cols_b));
    MPI_Recv(buffer.data(), src_row_count * cols_b, MPI_DOUBLE, src, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for (int idx = 0; idx < src_row_count; ++idx) {
      int global_row = src_rows[idx];
      for (int j = 0; j < cols_b; ++j) {
        final_result_flat[(static_cast<size_t>(global_row) * static_cast<size_t>(cols_b)) + static_cast<size_t>(j)] =
            buffer[(static_cast<size_t>(idx) * static_cast<size_t>(cols_b)) + static_cast<size_t>(j)];
      }
    }
  }
}

void SosninaAMatrixMultHorizontalMPI::SendLocalResults(const std::vector<double> &local_result_flat, int local_rows,
                                                       int cols_b) {
  if (local_rows > 0) {
    std::vector<double> data_copy = local_result_flat;
    MPI_Send(data_copy.data(), local_rows * cols_b, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);
  }
}

void SosninaAMatrixMultHorizontalMPI::GatherResults(std::vector<double> &final_result_flat,
                                                    const std::vector<int> &my_row_indices,
                                                    const std::vector<double> &local_result_flat, int local_rows,
                                                    int rows_a, int cols_b) const {
  if (rank_ == 0) {
    final_result_flat.resize(static_cast<size_t>(rows_a) * static_cast<size_t>(cols_b), 0.0);

    CollectLocalResults(my_row_indices, local_result_flat, final_result_flat, cols_b);

    for (int src = 1; src < world_size_; ++src) {
      ReceiveResultsFromProcess(src, final_result_flat, cols_b);
    }

    MPI_Bcast(final_result_flat.data(), rows_a * cols_b, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  } else {
    SendLocalResults(local_result_flat, local_rows, cols_b);

    final_result_flat.resize(static_cast<size_t>(rows_a) * static_cast<size_t>(cols_b));
    MPI_Bcast(final_result_flat.data(), rows_a * cols_b, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }
}

void SosninaAMatrixMultHorizontalMPI::ComputeLocalMultiplication(const std::vector<double> &local_a_flat,
                                                                 const std::vector<double> &b_flat,
                                                                 std::vector<double> &local_result_flat, int local_rows,
                                                                 int cols_a, int cols_b) {
  for (int i = 0; i < local_rows; ++i) {
    const double *a_row = &local_a_flat[static_cast<size_t>(i) * static_cast<size_t>(cols_a)];
    double *result_row = &local_result_flat[static_cast<size_t>(i) * static_cast<size_t>(cols_b)];

    for (int k = 0; k < cols_a; ++k) {
      double aik = a_row[k];
      const double *b_row = &b_flat[static_cast<size_t>(k) * static_cast<size_t>(cols_b)];

      for (int j = 0; j < cols_b; ++j) {
        result_row[j] += aik * b_row[j];
      }
    }
  }
}

void SosninaAMatrixMultHorizontalMPI::ConvertToMatrix(const std::vector<double> &final_result_flat, int rows_a,
                                                      int cols_b) {
  std::vector<std::vector<double>> result_matrix(rows_a, std::vector<double>(cols_b));

  for (int i = 0; i < rows_a; ++i) {
    for (int j = 0; j < cols_b; ++j) {
      result_matrix[i][j] =
          final_result_flat[(static_cast<size_t>(i) * static_cast<size_t>(cols_b)) + static_cast<size_t>(j)];
    }
  }

  GetOutput() = result_matrix;
}

bool SosninaAMatrixMultHorizontalMPI::PostProcessingImpl() {
  return true;
}

}  // namespace sosnina_a_matrix_mult_horizontal
