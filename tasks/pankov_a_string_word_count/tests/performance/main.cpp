#include <gtest/gtest.h>

#include <cstddef>
#include <sstream>

#include "pankov_a_string_word_count/common/include/common.hpp"
#include "pankov_a_string_word_count/mpi/include/ops_mpi.hpp"
#include "pankov_a_string_word_count/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace pankov_a_string_word_count {

class PankovARunPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;
  OutType expected_output_{};

  void SetUp() override {
    const size_t word_count = 10'000'000;
    std::ostringstream os;
    os.seekp(word_count * 6);
    os.clear();

    for (size_t i = 0; i < word_count; ++i) {
      os << "word ";
    }

    input_data_ = os.str();
    expected_output_ = word_count;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == expected_output_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(PankovARunPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, PankovAStringWordCountMPI, PankovAStringWordCountSEQ>(
    PPC_SETTINGS_pankov_a_string_word_count);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);
const auto kPerfTestName = PankovARunPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, PankovARunPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace pankov_a_string_word_count
