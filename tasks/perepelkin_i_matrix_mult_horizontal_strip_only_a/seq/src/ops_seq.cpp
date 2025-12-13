#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>

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

  return (GetOutput() == std::vector<std::vector<double>>());
}

bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::PreProcessingImpl() {
  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::RunImpl() {
  const auto &[matrix_a, matrix_b] = GetInput();

  const size_t height_a = matrix_a.size();
  const size_t width_a = matrix_a[0].size();
  const size_t width_b = matrix_b[0].size();

  auto &output = GetOutput();
  output = std::vector<std::vector<double>>(height_a, std::vector<double>(width_b, 0.0));

  double tmp;
  for (size_t i = 0; i < height_a; i++) {
    for (size_t j = 0; j < width_b; j++) {
      tmp = 0.0;
      for (size_t k = 0; k < width_a; k++) {
        tmp += matrix_a[i][k] * matrix_b[k][j];
      }
      output[i][j] = tmp;
    }
  }

  return true;
}

bool PerepelkinIMatrixMultHorizontalStripOnlyASEQ::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
