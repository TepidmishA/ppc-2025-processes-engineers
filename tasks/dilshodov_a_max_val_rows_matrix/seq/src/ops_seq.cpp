#include "dilshodov_a_max_val_rows_matrix/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>

#include "dilshodov_a_max_val_rows_matrix/common/include/common.hpp"

namespace dilshodov_a_max_val_rows_matrix {

MaxValRowsMatrixTaskSequential::MaxValRowsMatrixTaskSequential(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  InType copy = in;
  GetInput().swap(copy);
}

bool MaxValRowsMatrixTaskSequential::ValidationImpl() {
  const auto &input = GetInput();
  if (input.empty()) {
    return false;
  }
  std::size_t cols = input[0].size();
  if (cols == 0) {
    return false;
  }
  return std::ranges::all_of(input, [&](const auto &row) { return row.size() == cols; });
}

bool MaxValRowsMatrixTaskSequential::PreProcessingImpl() {
  GetOutput().assign(GetInput().size(), std::numeric_limits<int>::min());
  return true;
}

bool MaxValRowsMatrixTaskSequential::RunImpl() {
  const auto &input = GetInput();
  auto &output = GetOutput();
  int rows = static_cast<int>(input.size());

  for (int i = 0; i < rows; ++i) {
    output[i] = *std::max_element(input[i].begin(), input[i].end());
  }

  return true;
}

bool MaxValRowsMatrixTaskSequential::PostProcessingImpl() {
  return true;
}

}  // namespace dilshodov_a_max_val_rows_matrix
