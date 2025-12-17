#pragma once

#include "olesnitskiy_v_find_viol/common/include/common.hpp"
#include "task/include/task.hpp"

namespace olesnitskiy_v_find_viol {

class OlesnitskiyVFindViolSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit OlesnitskiyVFindViolSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace olesnitskiy_v_find_viol
