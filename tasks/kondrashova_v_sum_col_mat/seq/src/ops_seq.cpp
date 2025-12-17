#include "../include/ops_seq.hpp"

#include <cstddef>
#include <limits>
#include <vector>

#include "kondrashova_v_sum_col_mat/common/include/common.hpp"

namespace kondrashova_v_sum_col_mat {

KondrashovaVSumColMatSEQ::KondrashovaVSumColMatSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput().clear();
}

bool KondrashovaVSumColMatSEQ::ValidationImpl() {
  const auto &input = GetInput();

  return (input.size() >= 2) && GetOutput().empty() && (input[0] > 0) && (input[1] > 0) &&
         (input[0] <= std::numeric_limits<int>::max() / input[1]) &&
         (input.size() == (2 + (static_cast<size_t>(input[0]) * static_cast<size_t>(input[1]))));
}

bool KondrashovaVSumColMatSEQ::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool KondrashovaVSumColMatSEQ::RunImpl() {
  int rows = GetInput()[0];
  int cols = GetInput()[1];

  std::vector<int> col_sum_vec(cols, 0);

  for (int j = 0; j < cols; j++) {
    for (int i = 0; i < rows; i++) {
      int ind = 2 + (i * cols) + j;
      col_sum_vec[static_cast<std::size_t>(j)] += GetInput()[static_cast<std::size_t>(ind)];
    }
  }

  GetOutput() = col_sum_vec;
  return true;
}

bool KondrashovaVSumColMatSEQ::PostProcessingImpl() {
  return !GetOutput().empty();
}

}  // namespace kondrashova_v_sum_col_mat
