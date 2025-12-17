#include "olesnitskiy_v_striped_matrix_multiplication/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

#include "olesnitskiy_v_striped_matrix_multiplication/common/include/common.hpp"

namespace olesnitskiy_v_striped_matrix_multiplication {

OlesnitskiyVStripedMatrixMultiplicationMPI::OlesnitskiyVStripedMatrixMultiplicationMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = {0UL, 0UL, std::vector<double>()};
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size_);
}

std::vector<int> OlesnitskiyVStripedMatrixMultiplicationMPI::CalculateCounts(int total, int num_parts) {
  std::vector<int> counts(num_parts, 0);
  int base = total / num_parts;
  int remainder = total % num_parts;

  for (int i = 0; i < num_parts; ++i) {
    counts[i] = base + (i < remainder ? 1 : 0);
  }

  return counts;
}

std::vector<int> OlesnitskiyVStripedMatrixMultiplicationMPI::CalculateDisplacements(const std::vector<int> &counts) {
  std::vector<int> displs(counts.size(), 0);
  for (size_t i = 1; i < counts.size(); ++i) {
    displs[i] = displs[i - 1] + counts[i - 1];
  }
  return displs;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::ValidationImpl() {
  const auto &[rows_a, cols_a, data_a, rows_b, cols_b, data_b] = GetInput();
  rows_a_ = rows_a;
  cols_a_ = cols_a;
  data_a_ = data_a;
  rows_b_ = rows_b;
  cols_b_ = cols_b;
  data_b_ = data_b;
  if (rows_a == 0 || cols_a == 0 || rows_b == 0 || cols_b == 0) {
    return false;
  }

  if (data_a.size() != rows_a * cols_a || data_b.size() != rows_b * cols_b) {
    return false;
  }

  if (cols_a != rows_b) {
    return false;
  }
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::PreProcessingImpl() {
  rows_c_ = rows_a_;
  cols_c_ = cols_b_;
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::PrepareScatterData() {
  auto row_counts = CalculateCounts(static_cast<int>(rows_a_), world_size_);
  auto row_displs = CalculateDisplacements(row_counts);
  rows_a_local_ = row_counts[rank_];

  std::vector<int> sendcounts_a(world_size_);
  std::vector<int> displs_a(world_size_);
  for (int i = 0; i < world_size_; ++i) {
    sendcounts_a[i] = row_counts[i] * static_cast<int>(cols_a_);
    displs_a[i] = row_displs[i] * static_cast<int>(cols_a_);
  }

  sendcounts_a_ = std::move(sendcounts_a);
  displs_a_ = std::move(displs_a);
  row_counts_ = std::move(row_counts);
  row_displs_ = std::move(row_displs);

  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::ScatterData() {
  if (std::cmp_less(rows_a_, world_size_)) {
    return false;
  }

  if (!PrepareScatterData()) {
    return false;
  }

  if (rows_a_local_ > 0 && cols_a_ > 0) {
    local_a_.resize(static_cast<size_t>(rows_a_local_) * cols_a_);
  } else {
    local_a_.clear();
  }

  if (!local_a_.empty()) {
    MPI_Scatterv(data_a_.data(), sendcounts_a_.data(), displs_a_.data(), MPI_DOUBLE, local_a_.data(),
                 sendcounts_a_[rank_], MPI_DOUBLE, 0, MPI_COMM_WORLD);
  } else {
    MPI_Scatterv(data_a_.data(), sendcounts_a_.data(), displs_a_.data(), MPI_DOUBLE, nullptr, 0, MPI_DOUBLE, 0,
                 MPI_COMM_WORLD);
  }

  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::BroadcastMatrixB() {
  if (rows_b_ > 0 && cols_b_ > 0) {
    local_b_.resize(rows_b_ * cols_b_);
    if (rank_ == 0) {
      local_b_ = data_b_;
    }
    MPI_Bcast(local_b_.data(), static_cast<int>(local_b_.size()), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  } else {
    local_b_.clear();
  }
  return true;
}

void OlesnitskiyVStripedMatrixMultiplicationMPI::MultiplyRow(size_t row_start, size_t row_end) {
  for (size_t local_row = row_start; local_row < row_end; ++local_row) {
    for (size_t col = 0; col < cols_c_; ++col) {
      double sum = 0.0;
      for (size_t k = 0; k < cols_a_; ++k) {
        sum += local_a_[(local_row * cols_a_) + k] * local_b_[(k * cols_b_) + col];
      }
      local_c_[(local_row * cols_c_) + col] = sum;
    }
  }
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::ComputeLocalC() {
  if (rows_a_local_ > 0 && cols_c_ > 0 && cols_a_ > 0) {
    local_c_.resize(static_cast<size_t>(rows_a_local_) * cols_c_, 0.0);
    MultiplyRow(0, static_cast<size_t>(rows_a_local_));
  } else {
    local_c_.clear();
  }
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::PrepareGatherData() {
  recvcounts_c_.resize(world_size_);
  displs_c_.resize(world_size_);
  for (int i = 0; i < world_size_; ++i) {
    recvcounts_c_[i] = row_counts_[i] * static_cast<int>(cols_c_);
    displs_c_[i] = row_displs_[i] * static_cast<int>(cols_c_);
  }
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::GatherResults() {
  const size_t result_size = rows_c_ * cols_c_;

  if (!PrepareGatherData()) {
    return false;
  }

  if (rank_ == 0 && result_size > 0) {
    result_c_.resize(result_size, 0.0);
  }

  if (!local_c_.empty()) {
    MPI_Gatherv(local_c_.data(), recvcounts_c_[rank_], MPI_DOUBLE,
                rank_ == 0 && result_size > 0 ? result_c_.data() : nullptr, recvcounts_c_.data(), displs_c_.data(),
                MPI_DOUBLE, 0, MPI_COMM_WORLD);
  } else {
    MPI_Gatherv(nullptr, 0, MPI_DOUBLE, rank_ == 0 && result_size > 0 ? result_c_.data() : nullptr,
                recvcounts_c_.data(), displs_c_.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }

  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::BroadcastResultsFromRoot() {
  int result_rows = static_cast<int>(rows_c_);
  int result_cols = static_cast<int>(cols_c_);
  MPI_Bcast(&result_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&result_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (result_rows > 0 && result_cols > 0 && !result_c_.empty()) {
    MPI_Bcast(result_c_.data(), static_cast<int>(result_c_.size()), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }

  if (result_c_.empty()) {
    GetOutput() = {0UL, 0UL, std::vector<double>()};
  } else {
    GetOutput() = {rows_c_, cols_c_, result_c_};
  }

  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::ReceiveResultsFromRoot() {
  int result_rows = 0;
  int result_cols = 0;
  MPI_Bcast(&result_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&result_cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (result_rows > 0 && result_cols > 0) {
    std::vector<double> received_result(static_cast<size_t>(result_rows) * static_cast<size_t>(result_cols));
    MPI_Bcast(received_result.data(), static_cast<int>(received_result.size()), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    GetOutput() = {static_cast<size_t>(result_rows), static_cast<size_t>(result_cols), std::move(received_result)};
  } else {
    GetOutput() = {0UL, 0UL, std::vector<double>()};
  }

  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::BroadcastResults() {
  if (rank_ == 0) {
    return BroadcastResultsFromRoot();
  }

  return ReceiveResultsFromRoot();
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::SetOutput() {
  if (rank_ == 0) {
    if (result_c_.empty()) {
      GetOutput() = {0UL, 0UL, std::vector<double>()};
    } else {
      GetOutput() = {rows_c_, cols_c_, result_c_};
    }
  }
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::RunImpl() {
  if (std::cmp_less(rows_a_, world_size_)) {
    return RunOnSingleProcess();
  }

  if (!ScatterData()) {
    return false;
  }

  if (!BroadcastMatrixB()) {
    return false;
  }

  if (!ComputeLocalC()) {
    return false;
  }

  if (!GatherResults()) {
    return false;
  }

  if (!BroadcastResults()) {
    return false;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

void OlesnitskiyVStripedMatrixMultiplicationMPI::MultiplySingleProcessMatrix() {
  for (size_t i = 0; i < rows_a_; ++i) {
    for (size_t j = 0; j < cols_b_; ++j) {
      double sum = 0.0;
      for (size_t k = 0; k < cols_a_; ++k) {
        sum += data_a_[(i * cols_a_) + k] * data_b_[(k * cols_b_) + j];
      }
      result_c_[(i * cols_c_) + j] = sum;
    }
  }
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::ComputeSingleProcess() {
  const size_t result_size = rows_c_ * cols_c_;

  if (result_size > 0) {
    result_c_.resize(result_size, 0.0);
    MultiplySingleProcessMatrix();
  } else {
    result_c_.clear();
  }

  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::RunOnSingleProcess() {
  if (rank_ == 0) {
    if (!ComputeSingleProcess()) {
      return false;
    }

    if (!BroadcastResultsFromRoot()) {
      return false;
    }
  } else {
    if (!ReceiveResultsFromRoot()) {
      return false;
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationMPI::PostProcessingImpl() {
  return true;
}

}  // namespace olesnitskiy_v_striped_matrix_multiplication
