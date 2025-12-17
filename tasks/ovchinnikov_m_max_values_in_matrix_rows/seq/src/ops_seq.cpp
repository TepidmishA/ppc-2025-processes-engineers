#include "ovchinnikov_m_max_values_in_matrix_rows/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

#include "ovchinnikov_m_max_values_in_matrix_rows/common/include/common.hpp"

namespace ovchinnikov_m_max_values_in_matrix_rows {

OvchinnikovMMaxValuesInMatrixRowsSEQ::OvchinnikovMMaxValuesInMatrixRowsSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  static_cast<void>(GetOutput());
}

bool OvchinnikovMMaxValuesInMatrixRowsSEQ::ValidationImpl() {
  return true;
}

bool OvchinnikovMMaxValuesInMatrixRowsSEQ::PreProcessingImpl() {
  return true;
}

bool OvchinnikovMMaxValuesInMatrixRowsSEQ::RunImpl() {
  size_t lines = std::get<0>(GetInput());
  size_t cols = std::get<1>(GetInput());
  if (lines == 0 || cols == 0) {
    return true;
  }
  const auto &matrix = std::get<2>(GetInput());
  std::vector<int> result(cols, std::numeric_limits<int>::min());

  for (size_t i = 0; i < lines; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      result[j] = std::max(result[j], matrix[(i * cols) + j]);
    }
  }

  GetOutput() = result;
  return true;
}

bool OvchinnikovMMaxValuesInMatrixRowsSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace ovchinnikov_m_max_values_in_matrix_rows
