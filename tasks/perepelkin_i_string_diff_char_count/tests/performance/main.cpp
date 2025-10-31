#include <gtest/gtest.h>

#include "perepelkin_i_string_diff_char_count/common/include/common.hpp"
#include "perepelkin_i_string_diff_char_count/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_string_diff_char_count/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace perepelkin_i_string_diff_char_count {

class PerepelkinIStringDiffCharCountPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 100;
  InType input_data_{};

  void SetUp() override {
    input_data_ = kCount_;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return input_data_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(PerepelkinIStringDiffCharCountPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, PerepelkinIStringDiffCharCountMPI, PerepelkinIStringDiffCharCountSEQ>(PPC_SETTINGS_perepelkin_i_string_diff_char_count);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = PerepelkinIStringDiffCharCountPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, PerepelkinIStringDiffCharCountPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace perepelkin_i_string_diff_char_count
