#include "belov_e_lexico_order_two_strings/seq/include/ops_seq.hpp"

#include <string>
#include <vector>

#include "belov_e_lexico_order_two_strings/common/include/common.hpp"

namespace belov_e_lexico_order_two_strings {

BelovELexicoOrderTwoStringsSEQ::BelovELexicoOrderTwoStringsSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = false;
}

bool BelovELexicoOrderTwoStringsSEQ::ValidationImpl() {
  return !std::get<0>(GetInput()).empty() && !std::get<1>(GetInput()).empty();
}
bool BelovELexicoOrderTwoStringsSEQ::PreProcessingImpl() {
  std::vector<std::string> temp;
  std::string current;
  for (auto &ch : std::get<0>(GetInput())) {
    if (ch == ' ') {
      temp.push_back(current);
      current = "";
    } else {
      current += ch;
    }
  }
  if (!current.empty()) {
    temp.push_back(current);
  }
  std::get<0>(GetProccesedInput()) = temp;

  std::vector<std::string>().swap(temp);
  current = "";
  for (auto &ch : std::get<1>(GetInput())) {
    if (ch == ' ') {
      temp.push_back(current);
      current = "";
    } else {
      current += ch;
    }
  }
  if (!current.empty()) {
    temp.push_back(current);
  }
  std::get<1>(GetProccesedInput()) = temp;

  return !std::get<0>(GetProccesedInput()).empty() && !std::get<1>(GetProccesedInput()).empty();
}

bool BelovELexicoOrderTwoStringsSEQ::RunImpl() {
  const std::vector<std::string> &first = std::get<0>(GetProccesedInput());
  const std::vector<std::string> &second = std::get<1>(GetProccesedInput());

  bool flag = false;
  bool ans = false;
  auto iter1 = first.begin();
  auto iter2 = second.begin();
  for (; iter1 != first.end() && iter2 != second.end(); iter1++, iter2++) {
    if (*iter1 < *iter2) {
      ans = true;
      flag = true;
      break;
    }
    if (*iter1 != *iter2) {
      ans = false;
      flag = true;
      break;
    }
  }
  if (flag) {
    GetOutput() = ans;
  } else {
    GetOutput() = (iter1 == first.end()) && (iter2 != second.end());
  }

  return true;
}

bool BelovELexicoOrderTwoStringsSEQ::PostProcessingImpl() {
  return true;
}
}  // namespace belov_e_lexico_order_two_strings
