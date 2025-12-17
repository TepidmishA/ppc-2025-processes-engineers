#include <gtest/gtest.h>

#include <ctime>
#include <string>
#include <tuple>

#include "belov_e_lexico_order_two_strings/common/include/common.hpp"
#include "belov_e_lexico_order_two_strings/mpi/include/ops_mpi.hpp"
#include "belov_e_lexico_order_two_strings/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace belov_e_lexico_order_two_strings {

class BelovERunPerfTestsProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const size_t size_params_ = 3000;
  InType input_data_;
  OutType output_data_{};

  void SetUp() override {
    std::tuple<std::string, std::string, int> generated = GenerateDataForPerf(size_params_);
    input_data_ = std::make_tuple(std::get<0>(generated), std::get<1>(generated));
    output_data_ = static_cast<bool>(std::get<2>(generated));
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

  static std::tuple<std::string, std::string, int> GenerateDataForPerf(size_t size) {
    std::string word;
    for (size_t i = 0; i < size; ++i) {
      word += "a";
    }
    std::string str;
    for (size_t i = 0; i < size - 1; i++) {
      str += word + " ";
    }
    str += word;
    return std::make_tuple(str, str, 0);
  }
};

TEST_P(BelovERunPerfTestsProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, BelovELexicoOrderTwoStringsMPI, BelovELexicoOrderTwoStringsSEQ>(
        PPC_SETTINGS_belov_e_lexico_order_two_strings);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = BelovERunPerfTestsProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, BelovERunPerfTestsProcesses, kGtestValues, kPerfTestName);

}  // namespace belov_e_lexico_order_two_strings
