#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace shakirova_e_elem_matrix_sum {

struct Matrix {
  size_t rows;
  size_t cols;
  std::vector<int64_t> data;  // одномерный массив

  [[nodiscard]] bool IsValid() const {
    return rows > 0 && cols > 0 && data.size() == rows * cols;
  }

  [[nodiscard]] int64_t &At(size_t i, size_t j) {
    return data[(i * cols) + j];
  }

  [[nodiscard]] const int64_t &At(size_t i, size_t j) const {
    return data[(i * cols) + j];
  }

  friend bool operator==(const Matrix &v_left, const Matrix &v_right) {
    return v_left.rows == v_right.rows && v_left.cols == v_right.cols && v_left.data == v_right.data;
  }
};

}  // namespace shakirova_e_elem_matrix_sum
