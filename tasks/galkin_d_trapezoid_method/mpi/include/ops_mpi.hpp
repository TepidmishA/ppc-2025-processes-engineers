#pragma once

#include "galkin_d_trapezoid_method/common/include/common.hpp"
#include "task/include/task.hpp"

namespace galkin_d_trapezoid_method {

class GalkinDTrapezoidMethodMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit GalkinDTrapezoidMethodMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace galkin_d_trapezoid_method
