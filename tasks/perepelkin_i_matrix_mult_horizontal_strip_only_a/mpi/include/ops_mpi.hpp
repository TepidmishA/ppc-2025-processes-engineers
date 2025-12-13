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

  void PrepareRootData(int &height_a, int &width_a, int &width_b, std::vector<double> &flat_a,
                       std::vector<double> &flat_b);
  void BroadcastData(int &height_a, int &width_a, int &width_b, std::vector<double> &flat_b);
  int DistributeMatrixA(int height_a, int width_a, std::vector<double> &local_a, std::vector<int> &rows_per_rank,
                     const std::vector<double> &flat_a);
  void GatherAndBroadcastResult(int height_a, int width_b, const std::vector<int> &rows_per_rank,
                                const std::vector<double> &local_c, std::vector<double> &flat_c);
  void PopulateOutput(int height_a, int width_b, const std::vector<double> &flat_c);
};

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
