#include <gtest/gtest.h>

#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

#include "olesnitskiy_v_striped_matrix_multiplication/common/include/common.hpp"
#include "olesnitskiy_v_striped_matrix_multiplication/mpi/include/ops_mpi.hpp"
#include "olesnitskiy_v_striped_matrix_multiplication/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace olesnitskiy_v_striped_matrix_multiplication {

class OlesnitskiyVStripedMatrixMultiplicationPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    const size_t size = 1024;

    size_t rows_a = size;
    size_t cols_a = size;
    size_t rows_b = size;
    size_t cols_b = size;

    size_t total_a = rows_a * cols_a;
    size_t total_b = rows_b * cols_b;

    std::vector<double> matrix_a(total_a);
    std::vector<double> matrix_b(total_b);
    for (size_t i = 0; i < total_a; ++i) {
      matrix_a[i] = static_cast<double>((i * 37) % 1000) / 1000.0;
    }
    for (size_t i = 0; i < total_b; ++i) {
      matrix_b[i] = static_cast<double>((i * 73 + 50) % 1000) / 1000.0;
    }
    input_data_ = std::make_tuple(rows_a, cols_a, matrix_a, rows_b, cols_b, matrix_b);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const auto &[out_rows, out_cols, out_data] = output_data;
    return !out_data.empty() && out_rows == 1024 && out_cols == 1024;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(OlesnitskiyVStripedMatrixMultiplicationPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}
const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, OlesnitskiyVStripedMatrixMultiplicationMPI,
                                                       OlesnitskiyVStripedMatrixMultiplicationSEQ>(
    PPC_SETTINGS_olesnitskiy_v_striped_matrix_multiplication);
const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);
const auto kPerfTestName = OlesnitskiyVStripedMatrixMultiplicationPerfTests::CustomPerfTestName;
INSTANTIATE_TEST_SUITE_P(RunModeTests, OlesnitskiyVStripedMatrixMultiplicationPerfTests, kGtestValues, kPerfTestName);
}  // namespace olesnitskiy_v_striped_matrix_multiplication
