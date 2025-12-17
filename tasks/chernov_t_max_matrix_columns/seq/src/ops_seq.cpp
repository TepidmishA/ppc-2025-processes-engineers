#include "chernov_t_max_matrix_columns/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "chernov_t_max_matrix_columns/common/include/common.hpp"

namespace chernov_t_max_matrix_columns {

ChernovTMaxMatrixColumnsSEQ::ChernovTMaxMatrixColumnsSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool ChernovTMaxMatrixColumnsSEQ::ValidationImpl() {
  std::size_t m = std::get<0>(GetInput());
  std::size_t n = std::get<1>(GetInput());
  std::vector<int> &matrix = std::get<2>(GetInput());

  valid_ = (m > 0) && (n > 0) && (matrix.size() == m * n);
  return valid_;
}

bool ChernovTMaxMatrixColumnsSEQ::PreProcessingImpl() {
  if (!valid_) {
    return false;
  }

  rows_ = std::get<0>(GetInput());
  cols_ = std::get<1>(GetInput());
  input_matrix_ = std::get<2>(GetInput());

  return true;
}

bool ChernovTMaxMatrixColumnsSEQ::RunImpl() {
  if (!valid_) {
    return false;
  }

  std::vector<int> result(cols_);

  for (std::size_t col = 0; col < cols_; ++col) {
    int max_val = input_matrix_[col];

    for (std::size_t row = 1; row < rows_; ++row) {
      std::size_t index = (row * cols_) + col;
      max_val = std::max(input_matrix_[index], max_val);
    }
    result[col] = max_val;
  }

  GetOutput() = result;
  return true;
}

bool ChernovTMaxMatrixColumnsSEQ::PostProcessingImpl() {
  input_matrix_.clear();
  return true;
}

}  // namespace chernov_t_max_matrix_columns
