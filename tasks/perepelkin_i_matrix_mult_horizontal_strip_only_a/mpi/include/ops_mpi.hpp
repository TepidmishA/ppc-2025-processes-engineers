#pragma once

#include <cstddef>
#include <vector>

#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/common/include/common.hpp"
#include "task/include/task.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

class PerepelkinIMatrixMultHorizontalStripOnlyAMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit PerepelkinIMatrixMultHorizontalStripOnlyAMPI(const InType &in);

 private:
  int proc_rank_{};
  int proc_num_{};

  size_t height_a_{};
  size_t height_b_{};
  size_t width_a_{};
  size_t width_b_{};

  std::vector<double> flat_a_;
  std::vector<double> flat_b_t_;
  std::vector<double> flat_c_;

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  bool RootValidationImpl();
  static bool CheckConsistentRowWidths(const std::vector<std::vector<double>> &m, size_t expected_width);

  void BcastMatrixSizes();
  void BcastMatrixB();
  int DistributeMatrixA(std::vector<double> &local_a, std::vector<int> &rows_per_rank);
  void GatherAndBcastResult(const std::vector<int> &rows_per_rank, const std::vector<double> &local_c);
};

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
