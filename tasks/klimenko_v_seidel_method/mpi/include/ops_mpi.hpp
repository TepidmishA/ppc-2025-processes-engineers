#pragma once

#include <vector>

#include "klimenko_v_seidel_method/common/include/common.hpp"
#include "task/include/task.hpp"

namespace klimenko_v_seidel_method {

class KlimenkoVSeidelMethodMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KlimenkoVSeidelMethodMPI(const InType &in);

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static int ComputeFinalResult(const std::vector<double> &x, int n);
  static void InitializeMatrixAndVector(std::vector<double> &flat_matrix, std::vector<double> &b, int n);
  static void ComputeRowDistribution(int n, int size, std::vector<int> &row_counts, std::vector<int> &row_displs,
                                     std::vector<int> &matrix_counts, std::vector<int> &matrix_displs);
  static void PerformSeidelIteration(int local_rows, int start_row, int n, const std::vector<double> &local_matrix,
                                     const std::vector<double> &local_b, std::vector<double> &x);
  static double ComputeLocalDifference(int local_rows, int start_row, const std::vector<double> &x,
                                       const std::vector<double> &x_old);
  static void UpdateLocalXVector(int local_rows, int start_row, const std::vector<double> &x,
                                 std::vector<double> &local_x_updated);
};

}  // namespace klimenko_v_seidel_method
