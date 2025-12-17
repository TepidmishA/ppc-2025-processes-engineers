#include "olesnitskiy_v_striped_matrix_multiplication/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <vector>

#include "olesnitskiy_v_striped_matrix_multiplication/common/include/common.hpp"

namespace olesnitskiy_v_striped_matrix_multiplication {

namespace {
int FindCommonDivisor(int a, int b, int max_divisor) {
  if (a <= 0 || b <= 0 || max_divisor <= 1) {
    return 1;
  }

  for (int divisor = std::min({a, b, max_divisor}); divisor >= 1; --divisor) {
    if (a % divisor == 0 && b % divisor == 0) {
      return divisor;
    }
  }

  return 1;
}
}  // namespace

OlesnitskiyVStripedMatrixMultiplicationSEQ::OlesnitskiyVStripedMatrixMultiplicationSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::make_tuple(0, 0, std::vector<double>());
}

bool OlesnitskiyVStripedMatrixMultiplicationSEQ::ValidationImpl() {
  const auto &[rows_a, cols_a, data_a, rows_b, cols_b, data_b] = GetInput();
  const auto &[out_rows, out_cols, out_data] = GetOutput();
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
  if (out_rows != 0 || out_cols != 0 || !out_data.empty()) {
    return false;
  }

  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationSEQ::PreProcessingImpl() {
  rows_c_ = rows_a_;
  cols_c_ = cols_b_;
  int max_stripes = 8;
  num_stripes_ = FindCommonDivisor(static_cast<int>(rows_a_), static_cast<int>(cols_b_), max_stripes);
  if (num_stripes_ < 2) {
    num_stripes_ = 1;
  }
  result_c_.resize(rows_c_ * cols_c_, 0.0);
  GetOutput() = std::make_tuple(0, 0, std::vector<double>());
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationSEQ::RunImpl() {
  if (rows_a_ == 0 || cols_b_ == 0) {
    GetOutput() = std::make_tuple(rows_c_, cols_c_, std::vector<double>());
    return true;
  }

  result_c_.resize(rows_c_ * cols_c_, 0.0);

  bool success = false;
  if (num_stripes_ == 1) {
    success = MultiplySimple();
  } else {
    success = MultiplyStriped();
  }

  if (success) {
    GetOutput() = std::make_tuple(rows_c_, cols_c_, result_c_);
  }

  return success;
}

bool OlesnitskiyVStripedMatrixMultiplicationSEQ::MultiplySimple() {
  for (size_t i = 0; i < rows_a_; ++i) {
    for (size_t j = 0; j < cols_b_; ++j) {
      double sum = 0.0;
      for (size_t k = 0; k < cols_a_; ++k) {
        sum += data_a_[(i * cols_a_) + k] * data_b_[(k * cols_b_) + j];
      }
      result_c_[(i * cols_b_) + j] = sum;
    }
  }
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationSEQ::ProcessStripePair(int stripe_a, int stripe_b, size_t rows_per_stripe,
                                                                   size_t cols_per_stripe) {
  const size_t start_row_a = static_cast<size_t>(stripe_a) * rows_per_stripe;
  const size_t start_col_b = static_cast<size_t>(stripe_b) * cols_per_stripe;

  for (size_t i = 0; i < rows_per_stripe; ++i) {
    const size_t row_idx = start_row_a + i;
    for (size_t j = 0; j < cols_per_stripe; ++j) {
      const size_t col_idx = start_col_b + j;
      double sum = 0.0;

      for (size_t k = 0; k < cols_a_; ++k) {
        sum += data_a_[(row_idx * cols_a_) + k] * data_b_[(k * cols_b_) + col_idx];
      }
      result_c_[(row_idx * cols_b_) + col_idx] = sum;
    }
  }
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationSEQ::MultiplyStriped() {
  if (rows_a_ % static_cast<size_t>(num_stripes_) != 0 || cols_b_ % static_cast<size_t>(num_stripes_) != 0) {
    return false;
  }

  const size_t rows_per_stripe = rows_a_ / static_cast<size_t>(num_stripes_);
  const size_t cols_per_stripe = cols_b_ / static_cast<size_t>(num_stripes_);

  for (int stripe_a = 0; stripe_a < num_stripes_; ++stripe_a) {
    for (int stripe_b = 0; stripe_b < num_stripes_; ++stripe_b) {
      if (!ProcessStripePair(stripe_a, stripe_b, rows_per_stripe, cols_per_stripe)) {
        return false;
      }
    }
  }
  return true;
}

bool OlesnitskiyVStripedMatrixMultiplicationSEQ::PostProcessingImpl() {
  return true;
}
}  // namespace olesnitskiy_v_striped_matrix_multiplication
