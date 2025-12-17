#include "romanov_m_closest_elem_vec/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstddef>
#include <tuple>
#include <vector>

#include "romanov_m_closest_elem_vec/common/include/common.hpp"

namespace romanov_m_closest_elem_vec {

RomanovMClosestElemVecSEQ::RomanovMClosestElemVecSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::make_tuple(-1, -1);
  ;
}

bool RomanovMClosestElemVecSEQ::ValidationImpl() {
  return GetInput().size() >= 2;
}

bool RomanovMClosestElemVecSEQ::PreProcessingImpl() {
  return true;
}

bool RomanovMClosestElemVecSEQ::RunImpl() {
  const auto &v = GetInput();
  const size_t size = v.size();

  int min_diff = std::abs(v[1] - v[0]);
  int min_idx = 0;

  for (size_t i = 1; i < size - 1; ++i) {
    int current_diff = std::abs(v[i + 1] - v[i]);

    if (current_diff < min_diff) {
      min_diff = current_diff;
      min_idx = static_cast<int>(i);
    }
  }

  GetOutput() = std::make_tuple(min_idx, min_idx + 1);
  return true;
}

bool RomanovMClosestElemVecSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace romanov_m_closest_elem_vec
