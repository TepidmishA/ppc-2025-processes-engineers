#pragma once

#include <utility>
#include <vector>

#include "sosnina_a_matrix_mult_horizontal/common/include/common.hpp"
#include "task/include/task.hpp"

namespace sosnina_a_matrix_mult_horizontal {

using InTypeTriple =
    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>;  // входная пара матриц A и B

class SosninaAMatrixMultHorizontalSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }

  explicit SosninaAMatrixMultHorizontalSEQ(InTypeTriple in);

  [[nodiscard]] std::vector<std::vector<double>> GetResultMatrix() const;

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  InTypeTriple input_;
  std::vector<std::vector<double>> result_matrix_;
};

}  // namespace sosnina_a_matrix_mult_horizontal
