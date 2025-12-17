#include "bortsova_a_max_elem_vector/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>

#include "bortsova_a_max_elem_vector/common/include/common.hpp"

namespace bortsova_a_max_elem_vector {

BortsovaAMaxElemVectorSeq::BortsovaAMaxElemVectorSeq(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::numeric_limits<int>::min();
}

bool BortsovaAMaxElemVectorSeq::ValidationImpl() {
  return !GetInput().data.empty();
}

bool BortsovaAMaxElemVectorSeq::PreProcessingImpl() {
  GetOutput() = std::numeric_limits<int>::min();
  return true;
}

bool BortsovaAMaxElemVectorSeq::RunImpl() {
  const auto &vec = GetInput().data;

  if (vec.empty()) {
    return false;
  }

  int max_elem = vec[0];
  for (size_t i = 1; i < vec.size(); i++) {
    max_elem = std::max(vec[i], max_elem);
  }

  GetOutput() = max_elem;
  return true;
}

bool BortsovaAMaxElemVectorSeq::PostProcessingImpl() {
  return true;
}

}  // namespace bortsova_a_max_elem_vector
