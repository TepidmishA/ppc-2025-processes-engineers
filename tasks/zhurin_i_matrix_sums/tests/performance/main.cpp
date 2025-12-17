#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <vector>

#include "util/include/perf_test_util.hpp"
#include "zhurin_i_matrix_sums/common/include/common.hpp"
#include "zhurin_i_matrix_sums/mpi/include/ops_mpi.hpp"
#include "zhurin_i_matrix_sums/seq/include/ops_seq.hpp"

namespace zhurin_i_matrix_sums {

class ZhurinIMatrixSumsPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_{0, 0, {}};
  OutType expected_data_{0.0};

  void SetUp() override {
    uint32_t rows = 10000;
    uint32_t columns = 10000;
    uint64_t total_elements = static_cast<uint64_t>(rows) * columns;

    std::vector<double> inp(total_elements);

    for (uint64_t i = 0; i < total_elements; ++i) {
      inp[i] = static_cast<double>(i + 1);
    }

    double expected_value = static_cast<double>(total_elements) * (static_cast<double>(total_elements) + 1.0) / 2.0;

    input_data_ = InType(rows, columns, inp);
    expected_data_ = expected_value;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (expected_data_ == 0.0) {
      return output_data == 0.0;
    }
    double relative_error = std::abs(output_data - expected_data_) / std::abs(expected_data_);

    return relative_error < 1e-10;
  }

  [[nodiscard]] InType GetTestInputData() final {
    return input_data_;
  }
};

namespace {

TEST_P(ZhurinIMatrixSumsPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, ZhurinIMatrixSumsMPI, ZhurinIMatrixSumsSEQ>(PPC_SETTINGS_zhurin_i_matrix_sums);

inline const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

inline const auto kPerfTestName = ZhurinIMatrixSumsPerfTests::CustomPerfTestName;

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(ZhurinPerf, ZhurinIMatrixSumsPerfTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace zhurin_i_matrix_sums
