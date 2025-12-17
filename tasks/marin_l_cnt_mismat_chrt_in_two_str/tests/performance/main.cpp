#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "marin_l_cnt_mismat_chrt_in_two_str/common/include/common.hpp"
#include "marin_l_cnt_mismat_chrt_in_two_str/mpi/include/ops_mpi.hpp"
#include "marin_l_cnt_mismat_chrt_in_two_str/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace marin_l_cnt_mismat_chrt_in_two_str {

class MarinLCntMismatChrtInTwoStrPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 public:
  void SetUp() override {
    const int str_length = 40000000;
    std::string str1(str_length, 'A');
    std::string str2 = str1;

    for (int i = 0; i < str_length; i += 5) {
      str2[i] = 'B';
    }

    input_data_ = std::make_pair(str1, str2);
    expected_mismatches_ = str_length / 5;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == expected_mismatches_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  int expected_mismatches_{};
};

TEST_P(MarinLCntMismatChrtInTwoStrPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, MarinLCntMismatChrtInTwoStrMPI, MarinLCntMismatChrtInTwoStrSEQ>(
        PPC_SETTINGS_marin_l_cnt_mismat_chrt_in_two_str);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);
const auto kPerfTestName = MarinLCntMismatChrtInTwoStrPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, MarinLCntMismatChrtInTwoStrPerfTests, kGtestValues, kPerfTestName);

}  // namespace marin_l_cnt_mismat_chrt_in_two_str
