#include "perepelkin_i_string_diff_char_count/seq/include/ops_seq.hpp"

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
  const auto& [s1, s2] = GetInput();
  const size_t min_len = std::min(s1.size(), s2.size());
  const size_t max_len = std::max(s1.size(), s2.size());

  int diff = 0;
  for (size_t i = 0; i < min_len; ++i) {
    if (s1[i] != s2[i]) diff++;
  }

  diff += static_cast<int>(max_len - min_len);
  GetOutput() = diff;
  return true;
}

bool PerepelkinIStringDiffCharCountSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_string_diff_char_count
