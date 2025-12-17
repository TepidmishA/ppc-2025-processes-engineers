#include "sosnina_a_matrix_mult_horizontal/seq/include/ops_seq.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace sosnina_a_matrix_mult_horizontal {

SosninaAMatrixMultHorizontalSEQ::SosninaAMatrixMultHorizontalSEQ(InTypeTriple in) : input_(std::move(in)) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetOutput() = std::vector<std::vector<double>>();
}

bool SosninaAMatrixMultHorizontalSEQ::ValidationImpl() {
  const auto &matrix_a = input_.first;
  const auto &matrix_b = input_.second;

  if (matrix_a.empty() || matrix_b.empty()) {
    return false;
  }

  size_t cols_a = matrix_a[0].size();
  for (size_t i = 1; i < matrix_a.size(); i++) {
    if (matrix_a[i].size() != cols_a) {
      return false;
    }
  }

  size_t cols_b = matrix_b[0].size();
  for (size_t i = 1; i < matrix_b.size(); i++) {
    if (matrix_b[i].size() != cols_b) {
      return false;
    }
  }

  return cols_a == matrix_b.size();
}

bool SosninaAMatrixMultHorizontalSEQ::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool SosninaAMatrixMultHorizontalSEQ::RunImpl() {
  const auto &matrix_a = input_.first;
  const auto &matrix_b = input_.second;

  size_t rows_a = matrix_a.size();
  size_t cols_a = matrix_a[0].size();
  size_t cols_b = matrix_b[0].size();

  auto &output = GetOutput();
  output = std::vector<std::vector<double>>(rows_a, std::vector<double>(cols_b, 0.0));

  // Умножение матриц
  for (size_t i = 0; i < rows_a; i++) {
    for (size_t k = 0; k < cols_a; k++) {
      double aik = matrix_a[i][k];
      for (size_t j = 0; j < cols_b; j++) {
        output[i][j] += aik * matrix_b[k][j];
      }
    }
  }

  return true;
}

bool SosninaAMatrixMultHorizontalSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace sosnina_a_matrix_mult_horizontal
