#include <gtest/gtest.h>

#include <cstddef>
#include <random>
#include <tuple>
#include <utility>
#include <vector>

#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/common/include/common.hpp"
#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

class PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
 private:
  InType input_data_;
  OutType expected_;

  size_t rows_a_ = 1000;
  size_t cols_a_ = 2000;
  size_t cols_b_ = 2000;
  unsigned int seed_ = 42;

  void SetUp() override {
    const auto &[matrix_a, matrix_b, matrix_c] = GenerateTestData(rows_a_, cols_a_, cols_b_, seed_);
    input_data_ = std::make_pair(matrix_a, matrix_b);
    expected_ = matrix_c;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

  static std::tuple<std::vector<std::vector<double>>, std::vector<std::vector<double>>,
                    std::vector<std::vector<double>>>
  GenerateTestData(size_t rows_a, size_t cols_a, size_t cols_b, unsigned int seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<int> val_dist(-1000, 1000);

    std::vector<std::vector<double>> matrix_a(rows_a, std::vector<double>(cols_a));
    std::vector<std::vector<double>> matrix_b(cols_a, std::vector<double>(cols_b));

    for (size_t i = 0; i < rows_a; i++) {
      for (size_t j = 0; j < cols_a; j++) {
        matrix_a[i][j] = static_cast<double>(val_dist(gen));
      }
    }

    for (size_t i = 0; i < cols_a; i++) {
      for (size_t j = 0; j < cols_b; j++) {
        matrix_b[i][j] = static_cast<double>(val_dist(gen));
      }
    }

    std::vector<std::vector<double>> matrix_c(rows_a, std::vector<double>(cols_b, 0.0));

    for (size_t i = 0; i < rows_a; i++) {
      for (size_t j = 0; j < cols_b; j++) {
        double tmp = 0.0;
        for (size_t k = 0; k < cols_a; k++) {
          tmp += matrix_a[i][k] * matrix_b[k][j];
        }
        matrix_c[i][j] = tmp;
      }
    }

    return {matrix_a, matrix_b, matrix_c};
  }
};

TEST_P(PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, PerepelkinIMatrixMultHorizontalStripOnlyAMPI,
                                                       PerepelkinIMatrixMultHorizontalStripOnlyASEQ>(
    PPC_SETTINGS_perepelkin_i_matrix_mult_horizontal_strip_only_a);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses, kGtestValues,
                         kPerfTestName);

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
