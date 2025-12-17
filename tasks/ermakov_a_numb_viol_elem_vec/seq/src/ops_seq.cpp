#include "ermakov_a_numb_viol_elem_vec/seq/include/ops_seq.hpp"

#include <vector>

#include "ermakov_a_numb_viol_elem_vec/common/include/common.hpp"

namespace ermakov_a_numb_viol_elem_vec {

ErmakovANumbViolElemVecSEQ::ErmakovANumbViolElemVecSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

auto ErmakovANumbViolElemVecSEQ::ValidationImpl() -> bool {
  return true;
}

auto ErmakovANumbViolElemVecSEQ::PreProcessingImpl() -> bool {
  return true;
}

auto ErmakovANumbViolElemVecSEQ::RunImpl() -> bool {
  const auto &vec = GetInput();
  const int n = static_cast<int>(vec.size());
  if (n <= 0) {
    GetOutput() = 0;
    return true;
  }

  int viol = 0;
  for (int i = 0; i + 1 < n; ++i) {
    if (vec[i] > vec[i + 1]) {
      ++viol;
    }
  }

  GetOutput() = viol;
  return true;
}

auto ErmakovANumbViolElemVecSEQ::PostProcessingImpl() -> bool {
  return true;
}

}  // namespace ermakov_a_numb_viol_elem_vec
