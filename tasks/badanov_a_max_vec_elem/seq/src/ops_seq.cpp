#include "badanov_a_max_vec_elem/seq/include/ops_seq.hpp"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <vector>

#include "badanov_a_max_vec_elem/common/include/common.hpp"

namespace badanov_a_max_vec_elem {

BadanovAMaxVecElemSEQ::BadanovAMaxVecElemSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool BadanovAMaxVecElemSEQ::ValidationImpl() {
  return true;
}

bool BadanovAMaxVecElemSEQ::PreProcessingImpl() {
  return true;
}

bool BadanovAMaxVecElemSEQ::RunImpl() {
  if (GetInput().empty()) {
    GetOutput() = INT_MIN;
    return true;
  }

  int max_elem = GetInput()[0];
  for (size_t i = 1; i < GetInput().size(); i++) {
    max_elem = std::max(GetInput()[i], max_elem);
  }

  GetOutput() = max_elem;
  return true;
}

bool BadanovAMaxVecElemSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace badanov_a_max_vec_elem
