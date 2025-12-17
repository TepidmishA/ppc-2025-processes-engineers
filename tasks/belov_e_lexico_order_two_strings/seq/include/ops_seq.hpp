#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "belov_e_lexico_order_two_strings/common/include/common.hpp"
#include "task/include/task.hpp"

namespace belov_e_lexico_order_two_strings {
class BelovELexicoOrderTwoStringsSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit BelovELexicoOrderTwoStringsSEQ(const InType &in);

 private:
  std::tuple<std::vector<std::string>, std::vector<std::string>> proccesed_input_;
  std::tuple<std::vector<std::string>, std::vector<std::string>> &GetProccesedInput() {
    return proccesed_input_;
  }

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};
}  // namespace belov_e_lexico_order_two_strings
