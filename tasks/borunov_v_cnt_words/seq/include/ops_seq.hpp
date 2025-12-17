#pragma once

#include "borunov_v_cnt_words/common/include/common.hpp"
#include "task/include/task.hpp"

namespace borunov_v_cnt_words {

class BorunovVCntWordsSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit BorunovVCntWordsSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace borunov_v_cnt_words
