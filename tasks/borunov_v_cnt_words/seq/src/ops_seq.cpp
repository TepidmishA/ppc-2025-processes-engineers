#include <cctype>  // Для std::isspace
#include <string>  // Включаем, чтобы гарантировать, что std::string доступен

// ИСПРАВЛЕНИЕ ОШИБКИ C1083: Используем полный путь к заголовочному файлу
#include "borunov_v_cnt_words/common/include/common.hpp"
#include "borunov_v_cnt_words/seq/include/ops_seq.hpp"

namespace borunov_v_cnt_words {

BorunovVCntWordsSEQ::BorunovVCntWordsSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool BorunovVCntWordsSEQ::ValidationImpl() {
  return GetOutput() == 0;
}

bool BorunovVCntWordsSEQ::PreProcessingImpl() {
  return true;
}

bool BorunovVCntWordsSEQ::RunImpl() {
  const InType &str = GetInput();

  if (str.empty()) {
    GetOutput() = 0;
    return true;
  }

  OutType count = 0;
  bool in_word = false;

  for (char c : str) {
    if (std::isspace(static_cast<unsigned char>(c)) != 0) {
      in_word = false;
    } else {
      if (!in_word) {
        count++;
        in_word = true;
      }
    }
  }

  GetOutput() = count;
  return true;
}

bool BorunovVCntWordsSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace borunov_v_cnt_words
