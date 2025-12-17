#include "marin_l_cnt_mismat_chrt_in_two_str/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

#include "marin_l_cnt_mismat_chrt_in_two_str/common/include/common.hpp"

namespace marin_l_cnt_mismat_chrt_in_two_str {

MarinLCntMismatChrtInTwoStrSEQ::MarinLCntMismatChrtInTwoStrSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool MarinLCntMismatChrtInTwoStrSEQ::ValidationImpl() {
  return true;
}

bool MarinLCntMismatChrtInTwoStrSEQ::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

bool MarinLCntMismatChrtInTwoStrSEQ::RunImpl() {
  const std::string &s1 = GetInput().first;
  const std::string &s2 = GetInput().second;

  int count = 0;
  size_t min_len = std::min(s1.size(), s2.size());
  size_t max_len = std::max(s1.size(), s2.size());

  for (size_t i = 0; i < min_len; i++) {
    if (s1[i] != s2[i]) {
      count++;
    }
  }

  count += static_cast<int>(max_len - min_len);

  GetOutput() = count;
  return true;
}

bool MarinLCntMismatChrtInTwoStrSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace marin_l_cnt_mismat_chrt_in_two_str
