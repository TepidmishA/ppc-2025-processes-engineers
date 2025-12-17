#pragma once

#include <string>

#include "samoylenko_i_lex_order_check/common/include/common.hpp"
#include "task/include/task.hpp"

namespace samoylenko_i_lex_order_check {

class SamoylenkoILexOrderCheckSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }

  explicit SamoylenkoILexOrderCheckSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static bool SamoylenkoILexOrderCompare(const std::string &s1, const std::string &s2);
};

}  // namespace samoylenko_i_lex_order_check
