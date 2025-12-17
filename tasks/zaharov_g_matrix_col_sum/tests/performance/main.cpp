#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <vector>

#include "util/include/perf_test_util.hpp"
#include "zaharov_g_matrix_col_sum/common/include/common.hpp"
#include "zaharov_g_matrix_col_sum/mpi/include/ops_mpi.hpp"
#include "zaharov_g_matrix_col_sum/seq/include/ops_seq.hpp"

namespace zaharov_g_matrix_col_sum {

class ZaharovGMatrixColSumPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;
  std::vector<double> expected_sums_;

  void SetUp() override {
    GenerateLargeMatrix(10000);
  }

  void GenerateLargeMatrix(size_t size) {
    input_data_.resize(size);
    expected_sums_.resize(size, 0.0);

    for (size_t i = 0; i < size; ++i) {
      input_data_[i].resize(size);
      for (size_t j = 0; j < size; ++j) {
        auto value = static_cast<double>((i * size) + j + 1);
        input_data_[i][j] = value;
        expected_sums_[j] += value;
      }
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.empty() || output_data.size() != expected_sums_.size()) {
      return false;
    }

    for (size_t i = 0; i < output_data.size(); i++) {
      if (std::abs(output_data[i] - expected_sums_[i]) > 1) {
        return false;
      }
    }

    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(ZaharovGMatrixColSumPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, ZaharovGMatrixColSumMPI, ZaharovGMatrixColSumSEQ>(
    PPC_SETTINGS_zaharov_g_matrix_col_sum);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ZaharovGMatrixColSumPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ZaharovGMatrixColSumPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace zaharov_g_matrix_col_sum
