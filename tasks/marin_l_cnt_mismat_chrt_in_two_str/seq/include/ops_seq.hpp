#pragma once

#include "marin_l_cnt_mismat_chrt_in_two_str/common/include/common.hpp"
#include "task/include/task.hpp"

namespace marin_l_cnt_mismat_chrt_in_two_str {

class MarinLCntMismatChrtInTwoStrSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit MarinLCntMismatChrtInTwoStrSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace marin_l_cnt_mismat_chrt_in_two_str
