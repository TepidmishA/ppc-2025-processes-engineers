#include "klimenko_v_seidel_method/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <climits>
#include <cmath>
#include <cstddef>
#include <random>
#include <vector>

#include "klimenko_v_seidel_method/common/include/common.hpp"

namespace klimenko_v_seidel_method {

KlimenkoVSeidelMethodMPI::KlimenkoVSeidelMethodMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool KlimenkoVSeidelMethodMPI::ValidationImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int is_valid = 0;
  if (rank == 0) {
    is_valid = ((GetInput() > 0) && (GetOutput() == 0)) ? 1 : 0;
  }
  MPI_Bcast(&is_valid, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return is_valid != 0;
}

bool KlimenkoVSeidelMethodMPI::PreProcessingImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  GetOutput() = 0;

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool KlimenkoVSeidelMethodMPI::RunImpl() {
  int n = GetInput();

  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::vector<int> row_counts(size);
  std::vector<int> row_displs(size);
  std::vector<int> matrix_counts(size);
  std::vector<int> matrix_displs(size);
  ComputeRowDistribution(n, size, row_counts, row_displs, matrix_counts, matrix_displs);

  int local_rows = row_counts[rank];
  int start_row = row_displs[rank];

  std::vector<double> flat_matrix;
  std::vector<double> b;
  if (rank == 0) {
    InitializeMatrixAndVector(flat_matrix, b, n);
  }

  std::vector<double> local_matrix(static_cast<size_t>(local_rows) * n, 0.0);
  MPI_Scatterv(flat_matrix.data(), matrix_counts.data(), matrix_displs.data(), MPI_DOUBLE, local_matrix.data(),
               local_rows * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  std::vector<double> local_b(local_rows, 0.0);
  MPI_Scatterv(b.data(), row_counts.data(), row_displs.data(), MPI_DOUBLE, local_b.data(), local_rows, MPI_DOUBLE, 0,
               MPI_COMM_WORLD);

  std::vector<double> x(n, 0.0);
  const double epsilon = 1e-6;
  const int max_iterations = 1000;

  for (int iteration = 0; iteration < max_iterations; iteration++) {
    std::vector<double> x_old = x;

    PerformSeidelIteration(local_rows, start_row, n, local_matrix, local_b, x);

    std::vector<double> local_x_updated(local_rows);
    UpdateLocalXVector(local_rows, start_row, x, local_x_updated);

    MPI_Allgatherv(local_x_updated.data(), local_rows, MPI_DOUBLE, x.data(), row_counts.data(), row_displs.data(),
                   MPI_DOUBLE, MPI_COMM_WORLD);

    double local_diff = ComputeLocalDifference(local_rows, start_row, x, x_old);
    double global_diff = 0.0;
    MPI_Allreduce(&local_diff, &global_diff, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    global_diff = std::sqrt(global_diff);

    if (global_diff < epsilon) {
      break;
    }
  }

  if (rank == 0) {
    GetOutput() = ComputeFinalResult(x, n);
  }

  MPI_Bcast(&GetOutput(), 1, MPI_INT, 0, MPI_COMM_WORLD);
  return true;
}

bool KlimenkoVSeidelMethodMPI::PostProcessingImpl() {
  return GetOutput() > 0;
}

void KlimenkoVSeidelMethodMPI::ComputeRowDistribution(int n, int size, std::vector<int> &row_counts,
                                                      std::vector<int> &row_displs, std::vector<int> &matrix_counts,
                                                      std::vector<int> &matrix_displs) {
  int row_offset = 0;
  int matrix_offset = 0;
  for (int proc = 0; proc < size; proc++) {
    int base_rows = n / size;
    int extra = (proc < (n % size)) ? 1 : 0;
    int proc_rows = base_rows + extra;

    row_counts[proc] = proc_rows;
    row_displs[proc] = row_offset;
    matrix_counts[proc] = proc_rows * n;
    matrix_displs[proc] = matrix_offset;

    row_offset += proc_rows;
    matrix_offset += proc_rows * n;
  }
}

int KlimenkoVSeidelMethodMPI::ComputeFinalResult(const std::vector<double> &x, int n) {
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    sum += x[i];
  }
  return static_cast<int>(std::round(sum));
}

void KlimenkoVSeidelMethodMPI::InitializeMatrixAndVector(std::vector<double> &flat_matrix, std::vector<double> &b,
                                                         int n) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dist_off_diag(0.0, 1.0);
  std::uniform_int_distribution<int> dist_diag(10, 19);

  flat_matrix.resize(static_cast<size_t>(n) * n, 0.0);
  b.resize(n, 0.0);

  std::vector<double> x_exact(n, 1.0);

  for (int i = 0; i < n; i++) {
    double row_sum = 0.0;

    for (int j = 0; j < n; j++) {
      if (i == j) {
        flat_matrix[(static_cast<size_t>(i) * n) + j] = static_cast<double>(dist_diag(gen));
      } else {
        double val = dist_off_diag(gen);
        flat_matrix[(static_cast<size_t>(i) * n) + j] = val;
        row_sum += std::abs(val);
      }
    }

    if (std::abs(flat_matrix[(static_cast<size_t>(i) * n) + i]) < row_sum) {
      flat_matrix[(static_cast<size_t>(i) * n) + i] = row_sum + 1.0;
    }
  }

  for (int i = 0; i < n; i++) {
    double sum = 0.0;
    for (int j = 0; j < n; j++) {
      sum += flat_matrix[(static_cast<size_t>(i) * n) + j] * x_exact[j];
    }
    b[i] = sum;
  }
}

void KlimenkoVSeidelMethodMPI::PerformSeidelIteration(int local_rows, int start_row, int n,
                                                      const std::vector<double> &local_matrix,
                                                      const std::vector<double> &local_b, std::vector<double> &x) {
  for (int i = 0; i < local_rows; i++) {
    int global_i = start_row + i;
    double sum_off_diag = 0.0;

    for (int j = 0; j < n; j++) {
      if (j != global_i) {
        sum_off_diag += local_matrix[(static_cast<size_t>(i) * n) + j] * x[j];
      }
    }

    x[global_i] = (local_b[i] - sum_off_diag) / local_matrix[(static_cast<size_t>(i) * n) + global_i];
  }
}

void KlimenkoVSeidelMethodMPI::UpdateLocalXVector(int local_rows, int start_row, const std::vector<double> &x,
                                                  std::vector<double> &local_x_updated) {
  for (int i = 0; i < local_rows; ++i) {
    local_x_updated[i] = x[start_row + i];
  }
}

double KlimenkoVSeidelMethodMPI::ComputeLocalDifference(int local_rows, int start_row, const std::vector<double> &x,
                                                        const std::vector<double> &x_old) {
  double local_diff = 0.0;
  for (int i = 0; i < local_rows; i++) {
    int gi = start_row + i;
    double d = x[gi] - x_old[gi];
    local_diff += d * d;
  }
  return local_diff;
}

}  // namespace klimenko_v_seidel_method
