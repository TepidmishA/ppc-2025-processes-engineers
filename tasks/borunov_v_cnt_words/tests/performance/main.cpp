#include <gtest/gtest.h>

#include <cstddef>
#include <string>

#include "borunov_v_cnt_words/common/include/common.hpp"
#include "borunov_v_cnt_words/mpi/include/ops_mpi.hpp"
#include "borunov_v_cnt_words/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace borunov_v_cnt_words {

class BorunovVCntWordsPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 public:
  void SetUp() override {
    const size_t k_num_words = 25000000;
    const std::string k_word = "word ";

    input_data_.reserve(k_num_words * k_word.length());

    for (size_t i = 0; i < k_num_words; ++i) {
      input_data_ += k_word;
    }

    expected_count_ = k_num_words;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return expected_count_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_count_ = 0;
};

TEST_P(BorunovVCntWordsPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, BorunovVCntWordsMPI, BorunovVCntWordsSEQ>(PPC_SETTINGS_borunov_v_cnt_words);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = BorunovVCntWordsPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, BorunovVCntWordsPerfTests, kGtestValues, kPerfTestName);

}  // namespace borunov_v_cnt_words
