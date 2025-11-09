#include "perepelkin_i_string_diff_char_count/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>

#include "perepelkin_i_string_diff_char_count/common/include/common.hpp"

namespace perepelkin_i_string_diff_char_count {

PerepelkinIStringDiffCharCountSEQ::PerepelkinIStringDiffCharCountSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool PerepelkinIStringDiffCharCountSEQ::ValidationImpl() {
  return (GetOutput() == 0);
}

bool PerepelkinIStringDiffCharCountSEQ::PreProcessingImpl() {
  return true;
}

bool PerepelkinIStringDiffCharCountSEQ::RunImpl() {
  const auto &[s1, s2] = GetInput();
  const size_t min_len = std::min(s1.size(), s2.size());
  const size_t max_len = std::max(s1.size(), s2.size());

  int diff = std::transform_reduce(s1.begin(), s1.begin() + static_cast<std::ptrdiff_t>(min_len), s2.begin(), 0,
                                   std::plus<>(), std::not_equal_to<>());

  GetOutput() = diff + static_cast<int>(max_len - min_len);
  return true;
}

bool PerepelkinIStringDiffCharCountSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_string_diff_char_count
