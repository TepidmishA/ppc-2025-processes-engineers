#include "kondrashova_v_sum_col_mat/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

#include "kondrashova_v_sum_col_mat/common/include/common.hpp"

namespace kondrashova_v_sum_col_mat {

KondrashovaVSumColMatMPI::KondrashovaVSumColMatMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput().clear();
}

bool KondrashovaVSumColMatMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int is_valid = 0;

  if (rank == 0) {
    is_valid = static_cast<int>(
        (GetInput().size() >= 2) && GetOutput().empty() && (GetInput()[0] > 0) && (GetInput()[1] > 0) &&
        (GetInput()[0] <= std::numeric_limits<int>::max() / GetInput()[1]) &&
        (GetInput().size() == (2 + (static_cast<size_t>(GetInput()[0]) * static_cast<size_t>(GetInput()[1])))));
  }

  MPI_Bcast(&is_valid, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return static_cast<bool>(is_valid);
}

bool KondrashovaVSumColMatMPI::PreProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    rows_ = GetInput()[0];
    cols_ = GetInput()[1];
  }

  MPI_Bcast(&rows_, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&cols_, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    GetOutput().assign(static_cast<size_t>(cols_), 0);
  }

  return true;
}

namespace {

void ComputeLocalCols(int rank, int ost, int cols_on_proc, int &first_col, int &end_col) {
  if (rank < ost) {
    first_col = rank * (cols_on_proc + 1);
    end_col = first_col + (cols_on_proc + 1);
  } else {
    first_col = (ost * (cols_on_proc + 1)) + ((rank - ost) * cols_on_proc);
    end_col = first_col + cols_on_proc;
  }
}

void ComputeLocalSums(const std::vector<int> &matrix, std::vector<int> &local_sums, int rows, int cols, int first_col) {
  const int local_cols = static_cast<int>(local_sums.size());
  for (int col = 0; col < local_cols; col++) {
    const int global_col = first_col + col;
    for (int row = 0; row < rows; row++) {
      local_sums[col] += matrix[(row * cols) + global_col];
    }
  }
}

void GatherSums(std::vector<int> &local_sums, int first_col, int local_cols, int rank, int size,
                std::vector<int> &output) {
  if (rank == 0) {
    std::ranges::copy(local_sums, output.begin() + first_col);

    for (int src = 1; src < size; src++) {
      int src_first = 0;
      int src_local = 0;

      MPI_Recv(&src_first, 1, MPI_INT, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(&src_local, 1, MPI_INT, src, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      MPI_Recv(&output[src_first], src_local, MPI_INT, src, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

  } else {
    MPI_Send(&first_col, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    MPI_Send(&local_cols, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);

    MPI_Send(local_sums.data(), local_cols, MPI_INT, 0, 3, MPI_COMM_WORLD);
  }
}

}  // namespace

bool KondrashovaVSumColMatMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const int cols_on_proc = cols_ / size;
  const int ost = cols_ % size;

  int first_col = 0;
  int end_col = 0;
  ComputeLocalCols(rank, ost, cols_on_proc, first_col, end_col);

  const int local_cols = end_col - first_col;

  std::vector<int> matrix;
  if (rank == 0) {
    matrix.assign(GetInput().begin() + 2, GetInput().end());
  } else {
    matrix.resize(static_cast<size_t>(rows_) * static_cast<size_t>(cols_));
  }

  MPI_Bcast(matrix.data(), rows_ * cols_, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> local_sums(static_cast<size_t>(local_cols), 0);
  ComputeLocalSums(matrix, local_sums, rows_, cols_, first_col);
  GatherSums(local_sums, first_col, local_cols, rank, size, GetOutput());

  return true;
}

bool KondrashovaVSumColMatMPI::PostProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    return !GetOutput().empty();
  }
  return true;
}

}  // namespace kondrashova_v_sum_col_mat
