#pragma once

#include <cstddef>
#include <vector>

#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/common/include/common.hpp"
#include "task/include/task.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

class PerepelkinIMatrixMultHorizontalStripOnlyASEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit PerepelkinIMatrixMultHorizontalStripOnlyASEQ(const InType &in);

 private:
  std::vector<double> flat_a_;
  std::vector<double> flat_b_t_;
  size_t height_a_ = 0;
  size_t height_b_ = 0;
  size_t width_a_ = 0;
  size_t width_b_ = 0;

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
