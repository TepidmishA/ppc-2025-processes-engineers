#pragma once

#include "samoylenko_i_lex_order_check/common/include/common.hpp"
#include "task/include/task.hpp"

namespace samoylenko_i_lex_order_check {

class SamoylenkoILexOrderCheckMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }

  explicit SamoylenkoILexOrderCheckMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace samoylenko_i_lex_order_check
