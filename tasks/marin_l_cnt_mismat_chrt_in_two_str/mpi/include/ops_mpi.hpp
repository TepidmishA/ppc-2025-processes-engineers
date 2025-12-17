#pragma once

#include <string>

#include "marin_l_cnt_mismat_chrt_in_two_str/common/include/common.hpp"
#include "task/include/task.hpp"

namespace marin_l_cnt_mismat_chrt_in_two_str {

class MarinLCntMismatChrtInTwoStrMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit MarinLCntMismatChrtInTwoStrMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  std::string local_s1_;
  std::string local_s2_;
};

}  // namespace marin_l_cnt_mismat_chrt_in_two_str
