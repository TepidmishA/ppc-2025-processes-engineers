#include "posternak_a_count_different_char_in_two_lines/seq/include/ops_seq.hpp"

#include <cstddef>
#include <string>
#include <utility>

#include "posternak_a_count_different_char_in_two_lines/common/include/common.hpp"

namespace posternak_a_count_different_char_in_two_lines {

PosternakACountDifferentCharInTwoLinesSEQ::PosternakACountDifferentCharInTwoLinesSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool PosternakACountDifferentCharInTwoLinesSEQ::ValidationImpl() {
  std::pair<std::string, std::string> &lines = GetInput();
  std::string s1 = lines.first;
  std::string s2 = lines.second;
  return !s1.empty() && !s2.empty();
}

bool PosternakACountDifferentCharInTwoLinesSEQ::PreProcessingImpl() {
  return true;
}
bool PosternakACountDifferentCharInTwoLinesSEQ::RunImpl() {
  std::pair<std::string, std::string> &lines = GetInput();
  std::string s1 = lines.first;
  std::string s2 = lines.second;

  int diff_count = 0;
  size_t min = 0;
  size_t max = 0;
  size_t s1_len = s1.length();
  size_t s2_len = s2.length();
  if (s1_len >= s2_len) {
    min = s2_len;
    max = s1_len;
  } else {
    min = s1_len;
    max = s2_len;
  }
  for (size_t i = 0; i < min; i++) {
    if (s1[i] != s2[i]) {
      diff_count++;
    }
  }

  diff_count += static_cast<int>(max - min);
  GetOutput() = diff_count;
  return true;
}

bool PosternakACountDifferentCharInTwoLinesSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace posternak_a_count_different_char_in_two_lines
