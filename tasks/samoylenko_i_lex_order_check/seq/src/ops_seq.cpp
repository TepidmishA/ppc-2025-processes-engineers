#include "samoylenko_i_lex_order_check/seq/include/ops_seq.hpp"

#include <string>
#include <utility>

#include "samoylenko_i_lex_order_check/common/include/common.hpp"

namespace samoylenko_i_lex_order_check {

bool SamoylenkoILexOrderCheckSEQ::SamoylenkoILexOrderCompare(const std::string &s1, const std::string &s2) {
  auto first1 = s1.begin();
  auto last1 = s1.end();
  auto first2 = s2.begin();
  auto last2 = s2.end();

  for (; (first1 != last1) && (first2 != last2); ++first1, ++first2) {
    if (*first1 < *first2) {
      return true;
    }
    if (*first2 < *first1) {
      return false;
    }
  }

  // Equal and prefix check
  return ((first1 == last1) && (first2 == last2)) || ((first1 == last1) && (first2 != last2));
}

SamoylenkoILexOrderCheckSEQ::SamoylenkoILexOrderCheckSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = false;
}

bool SamoylenkoILexOrderCheckSEQ::ValidationImpl() {
  return true;
}

bool SamoylenkoILexOrderCheckSEQ::PreProcessingImpl() {
  return true;
}

bool SamoylenkoILexOrderCheckSEQ::RunImpl() {
  const auto &input = GetInput();
  const std::string &s1 = input.first;
  const std::string &s2 = input.second;

  bool result = SamoylenkoILexOrderCompare(s1, s2);

  GetOutput() = result;

  return true;
}

bool SamoylenkoILexOrderCheckSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace samoylenko_i_lex_order_check
