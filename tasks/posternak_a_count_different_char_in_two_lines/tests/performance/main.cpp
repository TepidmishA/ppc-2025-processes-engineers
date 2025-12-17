#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "posternak_a_count_different_char_in_two_lines/common/include/common.hpp"
#include "posternak_a_count_different_char_in_two_lines/mpi/include/ops_mpi.hpp"
#include "posternak_a_count_different_char_in_two_lines/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace posternak_a_count_different_char_in_two_lines {

class PosternakARunPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 10000000;
  std::string first_string_;
  std::string second_string_;
  InType input_data_;
  OutType expected_output_{};

  void SetUp() override {
    for (int i = 0; i < kCount_; i++) {
      first_string_ += "string";
      second_string_ += "strong";
    }
    input_data_ = std::make_pair(first_string_, second_string_);
    expected_output_ = kCount_;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return expected_output_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(PosternakARunPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, PosternakACountDifferentCharInTwoLinesMPI,
                                                       PosternakACountDifferentCharInTwoLinesSEQ>(
    PPC_SETTINGS_posternak_a_count_different_char_in_two_lines);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = PosternakARunPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, PosternakARunPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace posternak_a_count_different_char_in_two_lines
