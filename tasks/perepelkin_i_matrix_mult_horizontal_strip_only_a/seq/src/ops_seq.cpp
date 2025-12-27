#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/seq/include/ops_seq.hpp"

#include <cstddef>
#include <functional>
#include <numeric>
#include <vector>

#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/common/include/common.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

PerepelkinIMatrixMultHorizontalStripOnlyASEQ::PerepelkinIMatrixMultHorizontalStripOnlyASEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<std::vector<double>>();
}

bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::ValidationImpl() {
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

  return GetOutput().empty();
}

bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::PreProcessingImpl() {
  const auto &[matrix_a, matrix_b] = GetInput();

  // Store original sizes
  height_a_ = matrix_a.size();
  height_b_ = matrix_b.size();
  width_a_ = matrix_a[0].size();
  width_b_ = matrix_b[0].size();

  // Flatten matrix A
  flat_a_.reserve(width_a_ * height_a_);
  for (const auto &row : matrix_a) {
    flat_a_.insert(flat_a_.end(), row.begin(), row.end());
  }

  // Create transposed-and-flattened matrix B
  flat_b_t_.resize(width_b_ * height_b_);
  for (size_t row = 0; row < height_b_; row++) {
    for (size_t col = 0; col < width_b_; col++) {
      flat_b_t_[(col * height_b_) + row] = matrix_b[row][col];
    }
  }

  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::RunImpl() {
  auto &output = GetOutput();
  output.resize(height_a_);
  for (auto &row : output) {
    row.resize(width_b_);
  }

  for (size_t i = 0; i < height_a_; ++i) {
    const auto a_it = flat_a_.begin() + static_cast<DiffT>(i * width_a_);
    const auto a_end = a_it + static_cast<DiffT>(width_a_);
    for (size_t j = 0; j < width_b_; ++j) {
      const auto b_it = flat_b_t_.begin() + static_cast<DiffT>(j * width_a_);
      output[i][j] = std::transform_reduce(a_it, a_end, b_it, 0.0, std::plus<>(), std::multiplies<>());
    }
  }

  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
