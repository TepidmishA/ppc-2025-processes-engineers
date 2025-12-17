#include "chernov_t_max_matrix_columns/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include "chernov_t_max_matrix_columns/common/include/common.hpp"

namespace chernov_t_max_matrix_columns {

ChernovTMaxMatrixColumnsMPI::ChernovTMaxMatrixColumnsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool ChernovTMaxMatrixColumnsMPI::ValidationImpl() {
  auto &input = GetInput();
  std::size_t m = std::get<0>(input);
  std::size_t n = std::get<1>(input);
  std::vector<int> &matrix = std::get<2>(input);

  valid_ = (m > 0) && (n > 0) && (matrix.size() == m * n);
  return valid_;
}

bool ChernovTMaxMatrixColumnsMPI::PreProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (!valid_) {
    return false;
  }

  if (rank == 0) {
    auto &input = GetInput();
    rows_ = std::get<0>(input);
    cols_ = std::get<1>(input);
    input_matrix_ = std::get<2>(input);
  }

  return true;
}

bool ChernovTMaxMatrixColumnsMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (!valid_) {
    GetOutput() = std::vector<int>();
    return false;
  }

  BroadcastDimensions(rank);
  std::vector<int> local_data = ScatterMatrixData(rank, size);
  std::vector<int> local_maxima = ComputeLocalMaxima(rank, size, local_data);
  ComputeAndBroadcastResult(local_maxima);

  return true;
}

void ChernovTMaxMatrixColumnsMPI::BroadcastDimensions(int rank) {
  std::array<int, 2> dimensions{};
  if (rank == 0) {
    dimensions[0] = static_cast<int>(rows_);
    dimensions[1] = static_cast<int>(cols_);
  }
  MPI_Bcast(dimensions.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);
  total_rows_ = dimensions[0];
  total_cols_ = dimensions[1];
}

std::vector<int> ChernovTMaxMatrixColumnsMPI::ScatterMatrixData(int rank, int size) {
  int base_cols = total_cols_ / size;
  int remainder = total_cols_ % size;

  int my_cols = base_cols;
  if (rank < remainder) {
    my_cols++;
  }

  int my_elements = my_cols * total_rows_;
  std::vector<int> local_data(my_elements);

  std::vector<int> send_counts(size, 0);
  std::vector<int> displacements(size, 0);
  std::vector<int> reordered_data;

  if (rank == 0) {
    reordered_data.resize(static_cast<std::size_t>(total_rows_) * static_cast<std::size_t>(total_cols_));
    for (int col = 0; col < total_cols_; ++col) {
      for (int row = 0; row < total_rows_; ++row) {
        reordered_data[(static_cast<std::size_t>(col) * static_cast<std::size_t>(total_rows_)) +
                       static_cast<std::size_t>(row)] =
            input_matrix_[(static_cast<std::size_t>(row) * static_cast<std::size_t>(total_cols_)) +
                          static_cast<std::size_t>(col)];
      }
    }

    int current_displacement = 0;
    for (int i = 0; i < size; ++i) {
      int cols_for_i = base_cols;
      if (i < remainder) {
        cols_for_i++;
      }
      send_counts[i] = cols_for_i * total_rows_;
      displacements[i] = current_displacement;
      current_displacement += send_counts[i];
    }
  }

  MPI_Bcast(send_counts.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(displacements.data(), size, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    MPI_Scatterv(reordered_data.data(), send_counts.data(), displacements.data(), MPI_INT, local_data.data(),
                 my_elements, MPI_INT, 0, MPI_COMM_WORLD);
  } else {
    std::vector<int> dummy_sendbuf;
    MPI_Scatterv(dummy_sendbuf.data(), send_counts.data(), displacements.data(), MPI_INT, local_data.data(),
                 my_elements, MPI_INT, 0, MPI_COMM_WORLD);
  }

  return local_data;
}

std::vector<int> ChernovTMaxMatrixColumnsMPI::ComputeLocalMaxima(int rank, int size,
                                                                 const std::vector<int> &local_data) const {
  int base_cols = total_cols_ / size;
  int remainder = total_cols_ % size;

  int my_cols = base_cols;
  if (rank < remainder) {
    my_cols++;
  }

  std::vector<int> local_maxima(my_cols);

  for (int local_col = 0; local_col < my_cols; ++local_col) {
    int max_val = local_data[(static_cast<std::size_t>(local_col) * static_cast<std::size_t>(total_rows_))];

    for (int row = 1; row < total_rows_; ++row) {
      int element = local_data[(static_cast<std::size_t>(local_col) * static_cast<std::size_t>(total_rows_)) +
                               static_cast<std::size_t>(row)];
      max_val = std::max(element, max_val);
    }
    local_maxima[local_col] = max_val;
  }

  return local_maxima;
}

void ChernovTMaxMatrixColumnsMPI::ComputeAndBroadcastResult(const std::vector<int> &local_maxima) {
  int size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int base_cols = total_cols_ / size;
  int remainder = total_cols_ % size;

  std::vector<int> recv_counts(size);
  std::vector<int> displacements(size);

  int current_displacement = 0;
  for (int i = 0; i < size; ++i) {
    recv_counts[i] = base_cols;
    if (i < remainder) {
      recv_counts[i]++;
    }
    displacements[i] = current_displacement;
    current_displacement += recv_counts[i];
  }

  std::vector<int> result(total_cols_);

  MPI_Gatherv(local_maxima.data(), static_cast<int>(local_maxima.size()), MPI_INT, result.data(), recv_counts.data(),
              displacements.data(), MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Bcast(result.data(), total_cols_, MPI_INT, 0, MPI_COMM_WORLD);
  GetOutput() = result;
}

bool ChernovTMaxMatrixColumnsMPI::PostProcessingImpl() {
  input_matrix_.clear();
  return true;
}

}  // namespace chernov_t_max_matrix_columns
