#pragma once

#include <cstddef>
#include <vector>

#include "olesnitskiy_v_striped_matrix_multiplication/common/include/common.hpp"
#include "task/include/task.hpp"

namespace olesnitskiy_v_striped_matrix_multiplication {

class OlesnitskiyVStripedMatrixMultiplicationSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit OlesnitskiyVStripedMatrixMultiplicationSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  bool MultiplySimple();
  bool MultiplyStriped();
  bool ProcessStripePair(int stripe_a, int stripe_b, size_t rows_per_stripe, size_t cols_per_stripe);

  size_t rows_a_{0};
  size_t cols_a_{0};
  std::vector<double> data_a_;
  size_t rows_b_{0};
  size_t cols_b_{0};
  std::vector<double> data_b_;
  size_t rows_c_{0};
  size_t cols_c_{0};
  std::vector<double> result_c_;
  int num_stripes_{1};
};
}  // namespace olesnitskiy_v_striped_matrix_multiplication
