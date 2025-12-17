#include "shakirova_e_elem_matrix_sum/seq/include/ops_seq.hpp"

#include <cstddef>

#include "shakirova_e_elem_matrix_sum/common/include/common.hpp"
#include "shakirova_e_elem_matrix_sum/common/include/matrix.hpp"

namespace shakirova_e_elem_matrix_sum {

ShakirovaEElemMatrixSumSEQ::ShakirovaEElemMatrixSumSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool ShakirovaEElemMatrixSumSEQ::ValidationImpl() {
  return GetInput().IsValid();
}

bool ShakirovaEElemMatrixSumSEQ::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

bool ShakirovaEElemMatrixSumSEQ::RunImpl() {
  GetOutput() = 0;

  for (size_t i = 0; i < GetInput().rows; i++) {
    for (size_t j = 0; j < GetInput().cols; j++) {
      GetOutput() += GetInput().At(i, j);
    }
  }

  return true;
}

bool ShakirovaEElemMatrixSumSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace shakirova_e_elem_matrix_sum
