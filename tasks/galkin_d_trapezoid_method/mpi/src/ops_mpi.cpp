#include "galkin_d_trapezoid_method/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>

#include "galkin_d_trapezoid_method/common/include/common.hpp"

namespace galkin_d_trapezoid_method {

GalkinDTrapezoidMethodMPI::GalkinDTrapezoidMethodMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool GalkinDTrapezoidMethodMPI::ValidationImpl() {
  const auto &in = GetInput();
  return (in.n > 0) && (in.b > in.a);
}

bool GalkinDTrapezoidMethodMPI::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

bool GalkinDTrapezoidMethodMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  InType in = (rank == 0) ? GetInput() : InType{};

  MPI_Bcast(&in, sizeof(InType), MPI_BYTE, 0, MPI_COMM_WORLD);

  GetInput() = in;

  double a = in.a;
  double b = in.b;
  int n = in.n;

  if (n <= 0 || !(a < b)) {
    if (rank == 0) {
      GetOutput() = 0.0;
    }
    return true;
  }

  int base = n / size;
  int rem = n % size;

  int local_n = base + (rank < rem ? 1 : 0);

  int start_i = (rank * base) + std::min(rank, rem);
  int end_i = start_i + local_n;
  double h = (b - a) / static_cast<double>(n);

  double local_sum = 0.0;
  for (int i = start_i; i < end_i; ++i) {
    double x_left = a + (static_cast<double>(i) * h);
    double x_right = a + (static_cast<double>(i + 1) * h);

    double f_left = Function(x_left, in.func_id);
    double f_right = Function(x_right, in.func_id);

    local_sum += (f_left + f_right) * 0.5 * h;
  }

  double global_sum = 0.0;
  MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  MPI_Bcast(&global_sum, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  GetOutput() = global_sum;

  return true;
}

bool GalkinDTrapezoidMethodMPI::PostProcessingImpl() {
  return true;
}

}  // namespace galkin_d_trapezoid_method
