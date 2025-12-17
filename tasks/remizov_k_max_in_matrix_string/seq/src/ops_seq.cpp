#include "remizov_k_max_in_matrix_string/seq/include/ops_seq.hpp"

#include <algorithm>
#include <limits>
#include <vector>

#include "remizov_k_max_in_matrix_string/common/include/common.hpp"

namespace remizov_k_max_in_matrix_string {

RemizovKMaxInMatrixStringSEQ::RemizovKMaxInMatrixStringSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  InType tmp(in);
  GetInput().swap(tmp);
}

bool RemizovKMaxInMatrixStringSEQ::ValidationImpl() {
  return true;
}

bool RemizovKMaxInMatrixStringSEQ::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool RemizovKMaxInMatrixStringSEQ::RunImpl() {
  if (GetInput().empty()) {
    return true;
  }

  std::vector<int> result;
  for (const auto &row : GetInput()) {
    if (!row.empty()) {
      int max_val = *std::ranges::max_element(row);
      result.push_back(max_val);
    } else {
      result.push_back(std::numeric_limits<int>::min());
    }
  }

  GetOutput() = result;
  return true;
}

bool RemizovKMaxInMatrixStringSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace remizov_k_max_in_matrix_string
