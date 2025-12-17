#pragma once

#include <vector>

#include "balchunayte_z_dot_product/common/include/common.hpp"
#include "task/include/task.hpp"

namespace balchunayte_z_dot_product {

class BalchunayteZDotProductMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }

  explicit BalchunayteZDotProductMPI(const InType &in) {
    SetTypeOfTask(GetStaticTypeOfTask());
    GetInput() = in;
    GetOutput() = 0.0;
  }

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  std::vector<double> local_a_;
  std::vector<double> local_b_;
  int world_rank_{0};
  int world_size_{1};
  int local_size_{0};
};

}  // namespace balchunayte_z_dot_product
