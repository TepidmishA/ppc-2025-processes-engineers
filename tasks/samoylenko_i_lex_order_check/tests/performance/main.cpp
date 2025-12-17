#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <utility>

#include "samoylenko_i_lex_order_check/common/include/common.hpp"
#include "samoylenko_i_lex_order_check/mpi/include/ops_mpi.hpp"
#include "samoylenko_i_lex_order_check/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace samoylenko_i_lex_order_check {

class SamoylenkoRunPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  OutType expected_data_{};
  InType input_data_;

  void SetUp() override {
    std::string base_pattern = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string str;

    auto single_len = static_cast<size_t>(10e7);
    size_t repeat_times = 2;

    str.reserve(single_len * repeat_times);

    for (size_t rep = 0; rep < repeat_times; ++rep) {
      for (size_t i = 0; i < single_len; i++) {
        char c = base_pattern[i % base_pattern.length()];
        str += c;
      }
    }

    input_data_ = std::make_pair(str, str);
    expected_data_ = true;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == expected_data_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(SamoylenkoRunPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, SamoylenkoILexOrderCheckMPI, SamoylenkoILexOrderCheckSEQ>(
        PPC_SETTINGS_samoylenko_i_lex_order_check);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = SamoylenkoRunPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, SamoylenkoRunPerfTests, kGtestValues, kPerfTestName);

}  // namespace samoylenko_i_lex_order_check
