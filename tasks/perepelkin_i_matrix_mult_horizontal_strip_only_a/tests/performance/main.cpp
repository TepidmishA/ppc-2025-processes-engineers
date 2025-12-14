#include <gtest/gtest.h>

#include <cstddef>
#include <random>
#include <string>
#include <tuple>
#include <utility>

#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/common/include/common.hpp"
#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_matrix_mult_horizontal_strip_only_a/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace perepelkin_i_matrix_mult_horizontal_strip_only_a {

class PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
 private:
  InType input_data_ = std::pair(std::vector<std::vector<double>>(), std::vector<std::vector<double>>());
  OutType expected_count_ = std::vector<std::vector<double>>();

  size_t rows_a_ = 100;
  size_t cols_a_ = 100;
  size_t cols_b_ = 100;
  unsigned int seed_ = 42;

  void SetUp() override {
    auto [A, B, C] = GenerateTestData(rows_a_, cols_a_, cols_b_, seed_);
    input_data_ = std::make_pair(std::move(A), std::move(B));
    expected_count_ = std::move(C);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_count_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

  static std::tuple<std::vector<std::vector<double>>, std::vector<std::vector<double>>,
                    std::vector<std::vector<double>>>
  GenerateTestData(size_t rows_a, size_t cols_a, size_t cols_b, unsigned int seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<double> val_dist(-1000.0, 1000.0);

    std::vector<std::vector<double>> A(rows_a, std::vector<double>(cols_a));
    std::vector<std::vector<double>> B(cols_a, std::vector<double>(cols_b));

    for (size_t i = 0; i < rows_a; ++i) {
      for (size_t j = 0; j < cols_a; ++j) {
        A[i][j] = val_dist(gen);
      }
    }

    for (size_t i = 0; i < cols_a; ++i) {
      for (size_t j = 0; j < cols_b; ++j) {
        B[i][j] = val_dist(gen);
      }
    }

    std::vector<std::vector<double>> C(rows_a, std::vector<double>(cols_b, 0.0));

    for (size_t i = 0; i < rows_a; ++i) {
      for (size_t t = 0; t < cols_a; ++t) {
        double a = A[i][t];
        for (size_t j = 0; j < cols_b; ++j) {
          C[i][j] += a * B[t][j];
        }
      }
    }

    return {A, B, C};
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
