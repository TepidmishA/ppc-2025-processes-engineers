#include "zaharov_g_matrix_col_sum/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "zaharov_g_matrix_col_sum/common/include/common.hpp"

namespace zaharov_g_matrix_col_sum {

ZaharovGMatrixColSumSEQ::ZaharovGMatrixColSumSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  InType tmp(in);
  GetInput().swap(tmp);
}

bool ZaharovGMatrixColSumSEQ::ValidationImpl() {
  if (GetInput().empty()) {
    return true;
  }

  size_t cols = GetInput()[0].size();
  return std::all_of(GetInput().begin(), GetInput().end(), [cols](const auto &row) { return row.size() == cols; });
}

bool ZaharovGMatrixColSumSEQ::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool ZaharovGMatrixColSumSEQ::RunImpl() {
  if (GetInput().empty()) {
    return true;
  }

  OutType out(GetInput()[0].size());

  for (size_t i = 0; i < GetInput()[0].size(); i++) {
    double sum = 0;

    for (const auto &row : GetInput()) {
      sum += row[i];
    }

    out[i] = sum;
  }
  GetOutput() = out;
  return true;
}

bool ZaharovGMatrixColSumSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace zaharov_g_matrix_col_sum
