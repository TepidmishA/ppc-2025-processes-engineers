#include "pankov_a_string_word_count/seq/include/ops_seq.hpp"

#include <cctype>
#include <string>

#include "pankov_a_string_word_count/common/include/common.hpp"

namespace pankov_a_string_word_count {

PankovAStringWordCountSEQ::PankovAStringWordCountSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool PankovAStringWordCountSEQ::ValidationImpl() {
  return GetOutput() == 0;
}

bool PankovAStringWordCountSEQ::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

namespace {

OutType CountWordsInString(const std::string &s) {
  int count = 0;
  bool in_word = false;

  for (unsigned char uc : s) {
    if (std::isspace(uc) == 0) {
      if (!in_word) {
        in_word = true;
        ++count;
      }
    } else {
      in_word = false;
    }
  }

  return count;
}

}  // namespace

bool PankovAStringWordCountSEQ::RunImpl() {
  const std::string &s = GetInput();
  GetOutput() = CountWordsInString(s);
  return true;
}

bool PankovAStringWordCountSEQ::PostProcessingImpl() {
  return GetOutput() >= 0;
}

}  // namespace pankov_a_string_word_count
