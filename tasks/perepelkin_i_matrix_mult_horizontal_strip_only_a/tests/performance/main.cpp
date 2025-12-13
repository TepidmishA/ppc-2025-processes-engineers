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

  void SetUp() override {
    return;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_count_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

};

TEST_P(PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, PerepelkinIMatrixMultHorizontalStripOnlyAMPI, PerepelkinIMatrixMultHorizontalStripOnlyASEQ>(
        PPC_SETTINGS_perepelkin_i_matrix_mult_horizontal_strip_only_a);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, PerepelkinIMatrixMultHorizontalStripOnlyAPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace perepelkin_i_matrix_mult_horizontal_strip_only_a
