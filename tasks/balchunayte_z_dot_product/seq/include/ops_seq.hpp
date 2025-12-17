#pragma once

#include "balchunayte_z_dot_product/common/include/common.hpp"
#include "task/include/task.hpp"

namespace balchunayte_z_dot_product {

class BalchunayteZDotProductSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }

  explicit BalchunayteZDotProductSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace balchunayte_z_dot_product
