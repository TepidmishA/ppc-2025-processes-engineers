#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>

#include "ovchinnikov_m_bubble_sort/common/include/common.hpp"
#include "ovchinnikov_m_bubble_sort/mpi/include/ops_mpi.hpp"
#include "ovchinnikov_m_bubble_sort/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace ovchinnikov_m_bubble_sort {

class OvchinnikovMBubbleSortPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
 public:
  OvchinnikovMBubbleSortPerfTest() = default;

 protected:
  void SetUp() override {
    size_ = 100000;

    input_.resize(size_);
    for (size_t i = 0; i < size_; i++) {
      input_[i] = static_cast<int>((i * 37) % 100000);  // almost random
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == CalcExpected(input_);
  }

  InType GetTestInputData() final {
    return input_;
  }

 private:
  size_t size_ = 0;
  InType input_;

  static OutType CalcExpected(const InType &input) {
    OutType res = input;

    bool sorted = false;
    while (!sorted) {
      sorted = true;
      for (size_t i = 0; i + 1 < res.size(); i += 2) {
        if (res[i] > res[i + 1]) {
          std::swap(res[i], res[i + 1]);
          sorted = false;
        }
      }
      for (size_t i = 1; i + 1 < res.size(); i += 2) {
        if (res[i] > res[i + 1]) {
          std::swap(res[i], res[i + 1]);
          sorted = false;
        }
      }
    }

    return res;
  }
};

TEST_P(OvchinnikovMBubbleSortPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, OvchinnikovMBubbleSortMPI, OvchinnikovMBubbleSortSEQ>(
    PPC_SETTINGS_ovchinnikov_m_bubble_sort);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = OvchinnikovMBubbleSortPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, OvchinnikovMBubbleSortPerfTest, kGtestValues, kPerfTestName);

}  // namespace ovchinnikov_m_bubble_sort
