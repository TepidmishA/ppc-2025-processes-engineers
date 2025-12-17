#include "viderman_a_elem_vec_sum/seq/include/ops_seq.hpp"

#include <numeric>

#include "viderman_a_elem_vec_sum/common/include/common.hpp"

namespace viderman_a_elem_vec_sum {

VidermanAElemVecSumSEQ::VidermanAElemVecSumSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool VidermanAElemVecSumSEQ::ValidationImpl() {
  return (GetOutput() == 0.0);
}

bool VidermanAElemVecSumSEQ::PreProcessingImpl() {
  return true;
}

bool VidermanAElemVecSumSEQ::RunImpl() {
  const auto &input = GetInput();
  GetOutput() = std::accumulate(input.begin(), input.end(), 0.0);
  return true;
}

bool VidermanAElemVecSumSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace viderman_a_elem_vec_sum
