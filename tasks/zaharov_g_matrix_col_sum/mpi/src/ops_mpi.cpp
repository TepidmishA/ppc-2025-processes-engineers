#include "zaharov_g_matrix_col_sum/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

#include "zaharov_g_matrix_col_sum/common/include/common.hpp"

namespace zaharov_g_matrix_col_sum {

ZaharovGMatrixColSumMPI::ZaharovGMatrixColSumMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  InType tmp(in);
  GetInput().swap(tmp);
}

bool ZaharovGMatrixColSumMPI::ValidationImpl() {
  if (GetInput().empty()) {
    return true;
  }

  size_t cols = GetInput()[0].size();
  return std::all_of(GetInput().begin(), GetInput().end(), [cols](const auto &row) { return row.size() == cols; });
}

bool ZaharovGMatrixColSumMPI::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool ZaharovGMatrixColSumMPI::RunImpl() {
  const InType &in = GetInput();
  if (in.empty()) {
    return false;
  }

  int size = 0;
  int rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int column_amount = static_cast<int>(in[0].size());

  if (rank == 0) {
    for (int i = 1; i < size; i++) {
      std::vector<int> interval = CalcInterval(size, i, column_amount);
      MPI_Send(interval.data(), 2, MPI_INT, i, 0, MPI_COMM_WORLD);
    }

    std::vector<int> interval = CalcInterval(size, 0, column_amount);
    OutType elems = SumColValues(interval[0], interval[1]);

    GetOutput().resize(column_amount);
    for (size_t i = 0; i < elems.size(); i++) {
      GetOutput()[interval[0] + i] = elems[i];
    }

    MPI_Status status;
    for (int i = 1; i < size; i++) {
      int elems_size = 0;
      MPI_Recv(&elems_size, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);

      std::vector<int> remote_interval(2);
      MPI_Recv(remote_interval.data(), 2, MPI_INT, i, 3, MPI_COMM_WORLD, &status);

      OutType buf(elems_size);
      MPI_Recv(buf.data(), elems_size, MPI_DOUBLE, i, 2, MPI_COMM_WORLD, &status);

      for (size_t j = 0; j < buf.size(); j++) {
        GetOutput()[remote_interval[0] + j] = buf[j];
      }
    }
  } else {
    MPI_Status status;

    std::vector<int> buf(2);
    MPI_Recv(buf.data(), 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

    OutType elems = SumColValues(buf[0], buf[1]);
    int elems_size = static_cast<int>(elems.size());

    MPI_Send(buf.data(), 2, MPI_INT, 0, 3, MPI_COMM_WORLD);
    MPI_Send(&elems_size, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
    MPI_Send(elems.data(), elems_size, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
  }

  int total_size = 0;
  if (rank == 0) {
    total_size = static_cast<int>(GetOutput().size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    GetOutput().resize(total_size);
  }

  MPI_Bcast(GetOutput().data(), total_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  return true;
}

bool ZaharovGMatrixColSumMPI::PostProcessingImpl() {
  return true;
}

std::vector<int> ZaharovGMatrixColSumMPI::CalcInterval(int thread_amount, int rank, int column_amount) {
  int base_chunk = column_amount / thread_amount;
  int remainder = column_amount % thread_amount;

  int start = (rank * base_chunk) + std::min(rank, remainder);
  int end = start + base_chunk + (rank < remainder ? 1 : 0);

  return {start, end};  // end is exclusive
}

OutType ZaharovGMatrixColSumMPI::SumColValues(const int start, const int end) {
  OutType out;
  const InType &in = GetInput();

  if (start < 0 || std::cmp_greater(end, in[0].size()) || start >= end) {
    return out;
  }

  out.reserve(end - start);

  for (int col = start; col < end; col++) {
    double sum = 0.0;

    for (const auto &row : in) {
      sum += row[col];
    }

    out.push_back(sum);
  }

  return out;
}

}  // namespace zaharov_g_matrix_col_sum
