#include "afanasyev_a_elem_vec_avg/seq/include/ops_seq.hpp"

#include <cstdint>
#include <numeric>
#include <vector>

#include "afanasyev_a_elem_vec_avg/common/include/common.hpp"

namespace afanasyev_a_elem_vec_avg {

AfanasyevAElemVecAvgSEQ::AfanasyevAElemVecAvgSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool AfanasyevAElemVecAvgSEQ::ValidationImpl() {
  return true;
}

bool AfanasyevAElemVecAvgSEQ::PreProcessingImpl() {
  return true;
}

bool AfanasyevAElemVecAvgSEQ::RunImpl() {
  const InType &vec = GetInput();
  int n = static_cast<int>(vec.size());

  if (n == 0) {
    GetOutput() = 0.0;
    return true;
  }

  int64_t sum = std::accumulate(vec.begin(), vec.end(), static_cast<int64_t>(0));

  GetOutput() = static_cast<OutType>(sum) / static_cast<double>(n);

  return true;
}

bool AfanasyevAElemVecAvgSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace afanasyev_a_elem_vec_avg
