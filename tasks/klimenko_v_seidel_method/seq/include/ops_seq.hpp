#pragma once

#include <vector>

#include "klimenko_v_seidel_method/common/include/common.hpp"
#include "task/include/task.hpp"

namespace klimenko_v_seidel_method {

class KlimenkoVSeidelMethodSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KlimenkoVSeidelMethodSEQ(const InType &in);

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

 private:
  std::vector<std::vector<double>> A_;
  std::vector<double> b_;
  std::vector<double> x_;
  int n_{0};
  double epsilon_{0.0};
  int max_iterations_{0};

  static void GenerateRandomMatrix(int size, std::vector<std::vector<double>> &matrix, std::vector<double> &vector);
  static double PerformSeidelIteration(int n, const std::vector<std::vector<double>> &a, const std::vector<double> &b,
                                       std::vector<double> &x);
  static bool CheckDiagonalElements(int n, const std::vector<std::vector<double>> &a);
  static void ComputeRightHandSide(int n, const std::vector<std::vector<double>> &a, std::vector<double> &b);
};

}  // namespace klimenko_v_seidel_method
