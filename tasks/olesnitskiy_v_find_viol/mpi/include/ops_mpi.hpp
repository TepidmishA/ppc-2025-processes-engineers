#pragma once

#include "olesnitskiy_v_find_viol/common/include/common.hpp"
#include "task/include/task.hpp"

namespace olesnitskiy_v_find_viol {

class OlesnitskiyVFindViolMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit OlesnitskiyVFindViolMPI(const InType &in);
  bool RunSequentialCase();

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  [[nodiscard]] static int CountViolation(double current, double next);
};

}  // namespace olesnitskiy_v_find_viol
