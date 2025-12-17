#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "safronov_m_bubble_sort_odd_even/common/include/common.hpp"
#include "safronov_m_bubble_sort_odd_even/mpi/include/ops_mpi.hpp"
#include "safronov_m_bubble_sort_odd_even/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace safronov_m_bubble_sort_odd_even {

class SafronovMBubbleSortOddEvenPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 50000;
  InType input_data_;
  OutType res_;

  void SetUp() override {
    std::vector<int> vec(kCount_);
    for (int i = 0; i < kCount_; i++) {
      vec[i] = kCount_ - i;
    }
    input_data_ = vec;
    // std::sort(vec.begin(), vec.end());
    std::ranges::sort(vec);
    res_ = vec;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return res_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(SafronovMBubbleSortOddEvenPerfTests, BubbleSortOddEvenPerf) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, SafronovMBubbleSortOddEvenMPI, SafronovMBubbleSortOddEvenSEQ>(
        PPC_SETTINGS_safronov_m_bubble_sort_odd_even);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = SafronovMBubbleSortOddEvenPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(BubbleSortOddEvenPerf, SafronovMBubbleSortOddEvenPerfTests, kGtestValues, kPerfTestName);

}  // namespace safronov_m_bubble_sort_odd_even
