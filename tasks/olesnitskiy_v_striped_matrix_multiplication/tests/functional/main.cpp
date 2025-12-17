#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "olesnitskiy_v_striped_matrix_multiplication/common/include/common.hpp"
#include "olesnitskiy_v_striped_matrix_multiplication/mpi/include/ops_mpi.hpp"
#include "olesnitskiy_v_striped_matrix_multiplication/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace olesnitskiy_v_striped_matrix_multiplication {

class OlesnitskiyVStripedMatrixMultiplicationFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<2>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    const auto &expected_output = std::get<1>(params);

    const auto &[out_rows, out_cols, out_data] = output_data;
    const auto &[exp_rows, exp_cols, exp_data] = expected_output;

    if (out_rows != exp_rows || out_cols != exp_cols) {
      return false;
    }

    const double epsilon = 1e-6;
    for (size_t i = 0; i < out_data.size(); ++i) {
      if (std::fabs(out_data[i] - exp_data[i]) > epsilon) {
        return false;
      }
    }

    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(OlesnitskiyVStripedMatrixMultiplicationFuncTests, MatrixMultiplication) {
  ExecuteTest(GetParam());
}

std::vector<double> CreateMatrix(size_t rows, size_t cols, double start_value = 1.0) {
  std::vector<double> matrix(rows * cols);
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      matrix[(i * cols) + j] = start_value + static_cast<double>((i * cols) + j);
    }
  }
  return matrix;
}

std::vector<double> MultiplyMatrices(const std::vector<double> &a, size_t rows_a, size_t cols_a,
                                     const std::vector<double> &b, size_t rows_b, size_t cols_b) {
  (void)rows_b;
  std::vector<double> c(rows_a * cols_b, 0.0);

  for (size_t i = 0; i < rows_a; ++i) {
    for (size_t j = 0; j < cols_b; ++j) {
      double sum = 0.0;
      for (size_t k = 0; k < cols_a; ++k) {
        sum += a[(i * cols_a) + k] * b[(k * cols_b) + j];
      }
      c[(i * cols_b) + j] = sum;
    }
  }

  return c;
}

const std::array<TestType, 15> kTestParam = {
    std::make_tuple(std::make_tuple(1UL, 1UL, std::vector<double>{2.0}, 1UL, 1UL, std::vector<double>{3.0}),
                    std::make_tuple(1UL, 1UL, std::vector<double>{6.0}), "1x1_single_element"),

    std::make_tuple(std::make_tuple(2UL, 2UL, std::vector<double>{1.0, 2.0, 3.0, 4.0}, 2UL, 2UL,
                                    std::vector<double>{2.0, 0.0, 1.0, 2.0}),
                    std::make_tuple(2UL, 2UL, std::vector<double>{4.0, 4.0, 10.0, 8.0}), "2x2_basic"),

    std::make_tuple(std::make_tuple(3UL, 3UL, std::vector<double>{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, 3UL,
                                    3UL, std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0}),
                    std::make_tuple(3UL, 3UL, std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0}),
                    "3x3_identity"),

    std::make_tuple(
        std::make_tuple(
            4UL, 4UL,
            std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0},
            4UL, 4UL,
            std::vector<double>{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0}),
        std::make_tuple(
            4UL, 4UL,
            std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0}),
        "4x4_identity_mult"),

    std::make_tuple(std::make_tuple(2UL, 3UL, std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0}, 3UL, 2UL,
                                    std::vector<double>{7.0, 8.0, 9.0, 10.0, 11.0, 12.0}),
                    std::make_tuple(2UL, 2UL, std::vector<double>{58.0, 64.0, 139.0, 154.0}), "2x3_x_3x2_rectangular"),

    std::make_tuple(
        std::make_tuple(5UL, 5UL, CreateMatrix(5, 5, 1.0), 5UL, 5UL, CreateMatrix(5, 5, 0.1)),
        std::make_tuple(5UL, 5UL, MultiplyMatrices(CreateMatrix(5, 5, 1.0), 5, 5, CreateMatrix(5, 5, 0.1), 5, 5)),
        "5x5_random_values"),

    std::make_tuple(
        std::make_tuple(8UL, 8UL, CreateMatrix(8, 8, 1.0), 8UL, 8UL, CreateMatrix(8, 8, 2.0)),
        std::make_tuple(8UL, 8UL, MultiplyMatrices(CreateMatrix(8, 8, 1.0), 8, 8, CreateMatrix(8, 8, 2.0), 8, 8)),
        "8x8_large_multiple_of_4"),

    std::make_tuple(
        std::make_tuple(3UL, 4UL, std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0},
                        4UL, 2UL, std::vector<double>{1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0}),
        std::make_tuple(3UL, 2UL, std::vector<double>{4.0, 6.0, 12.0, 14.0, 20.0, 22.0}), "3x4_x_4x2_non_square"),

    std::make_tuple(
        std::make_tuple(6UL, 6UL, CreateMatrix(6, 6, 0.5), 6UL, 6UL, CreateMatrix(6, 6, 2.0)),
        std::make_tuple(6UL, 6UL, MultiplyMatrices(CreateMatrix(6, 6, 0.5), 6, 6, CreateMatrix(6, 6, 2.0), 6, 6)),
        "6x6_multiple_of_2_not_4"),

    std::make_tuple(
        std::make_tuple(10UL, 10UL, CreateMatrix(10, 10, 0.1), 10UL, 10UL, CreateMatrix(10, 10, 10.0)),
        std::make_tuple(10UL, 10UL,
                        MultiplyMatrices(CreateMatrix(10, 10, 0.1), 10, 10, CreateMatrix(10, 10, 10.0), 10, 10)),
        "10x10_large_not_multiple"),

    std::make_tuple(std::make_tuple(3UL, 3UL, std::vector<double>(9, 0.0), 3UL, 3UL, CreateMatrix(3, 3, 1.0)),
                    std::make_tuple(3UL, 3UL, std::vector<double>(9, 0.0)), "3x3_zero_matrix"),

    std::make_tuple(
        std::make_tuple(4UL, 6UL, CreateMatrix(4, 6, 1.0), 6UL, 3UL, CreateMatrix(6, 3, 2.0)),
        std::make_tuple(4UL, 3UL, MultiplyMatrices(CreateMatrix(4, 6, 1.0), 4, 6, CreateMatrix(6, 3, 2.0), 6, 3)),
        "4x6_x_6x3_large_rectangular"),

    std::make_tuple(
        std::make_tuple(12UL, 12UL, CreateMatrix(12, 12, 1.0), 12UL, 12UL, CreateMatrix(12, 12, -1.0)),
        std::make_tuple(12UL, 12UL,
                        MultiplyMatrices(CreateMatrix(12, 12, 1.0), 12, 12, CreateMatrix(12, 12, -1.0), 12, 12)),
        "12x12_negative_values"),

    std::make_tuple(
        std::make_tuple(7UL, 7UL, CreateMatrix(7, 7, 0.5), 7UL, 7UL, CreateMatrix(7, 7, 4.0)),
        std::make_tuple(7UL, 7UL, MultiplyMatrices(CreateMatrix(7, 7, 0.5), 7, 7, CreateMatrix(7, 7, 4.0), 7, 7)),
        "7x7_prime_size"),

    std::make_tuple(
        std::make_tuple(9UL, 9UL, CreateMatrix(9, 9, 0.2), 9UL, 9UL, CreateMatrix(9, 9, 5.0)),
        std::make_tuple(9UL, 9UL, MultiplyMatrices(CreateMatrix(9, 9, 0.2), 9, 9, CreateMatrix(9, 9, 5.0), 9, 9)),
        "9x9_multiple_of_3")};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<OlesnitskiyVStripedMatrixMultiplicationMPI, InType>(
                                               kTestParam, PPC_SETTINGS_olesnitskiy_v_striped_matrix_multiplication),
                                           ppc::util::AddFuncTask<OlesnitskiyVStripedMatrixMultiplicationSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_olesnitskiy_v_striped_matrix_multiplication));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = OlesnitskiyVStripedMatrixMultiplicationFuncTests::PrintFuncTestName<
    OlesnitskiyVStripedMatrixMultiplicationFuncTests>;

INSTANTIATE_TEST_SUITE_P(MatrixMultiplicationTests, OlesnitskiyVStripedMatrixMultiplicationFuncTests, kGtestValues,
                         kPerfTestName);

}  // namespace

}  // namespace olesnitskiy_v_striped_matrix_multiplication
