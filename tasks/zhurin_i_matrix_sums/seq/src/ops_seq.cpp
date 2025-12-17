#include "zhurin_i_matrix_sums/seq/include/ops_seq.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "zhurin_i_matrix_sums/common/include/common.hpp"

namespace zhurin_i_matrix_sums {

ZhurinIMatrixSumsSEQ::ZhurinIMatrixSumsSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool ZhurinIMatrixSumsSEQ::ValidationImpl() {
  return (static_cast<uint64_t>(std::get<0>(GetInput())) * std::get<1>(GetInput()) == std::get<2>(GetInput()).size()) &&
         (GetOutput() == 0.0);
}

bool ZhurinIMatrixSumsSEQ::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

bool ZhurinIMatrixSumsSEQ::RunImpl() {
  auto rows = std::get<0>(GetInput());
  auto columns = std::get<1>(GetInput());
  const auto &matrix = std::get<2>(GetInput());

  double sum = 0.0;
  for (size_t i = 0; i < static_cast<size_t>(rows) * columns; i++) {
    sum += matrix[i];
  }

  GetOutput() = sum;
  return true;
}

bool ZhurinIMatrixSumsSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace zhurin_i_matrix_sums
