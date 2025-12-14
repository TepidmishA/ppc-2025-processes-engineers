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

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void BcastMatrixSizes(size_t &height_a, size_t &width_a, size_t &height_b, size_t &width_b);
  void BcastMatrixB(const size_t &height_b, const size_t &width_b, std::vector<double> &flat_b);
  int DistributeMatrixA(const size_t &height_a, const size_t &width_a, std::vector<double> &local_a,
                        std::vector<int> &rows_per_rank);
  void GatherAndBcastResult(const size_t &height_a, const size_t &width_b, const std::vector<int> &rows_per_rank,
                            const std::vector<double> &local_c, std::vector<double> &flat_c);
  void PrepareOutput(const size_t &height_a, const size_t &width_b, const std::vector<double> &flat_c);
};

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
