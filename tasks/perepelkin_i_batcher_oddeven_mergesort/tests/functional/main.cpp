#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "perepelkin_i_batcher_oddeven_mergesort/common/include/common.hpp"
#include "perepelkin_i_batcher_oddeven_mergesort/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_batcher_oddeven_mergesort/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace perepelkin_i_batcher_oddeven_mergesort {

class PerepelkinIBatcherOddEvenMergeSortFuncTests
    : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return test_param.first;
  }

 protected:
  void SetUp() override {
    const TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = params.second;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    OutType expected = input_data_;
    std::sort(expected.begin(), expected.end());
    return expected == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(PerepelkinIBatcherOddEvenMergeSortFuncTests, SortsCorrectly) { ExecuteTest(GetParam()); }

const std::array<TestType, 7> kTestParams = {
    std::make_pair("empty", std::vector<double>{}),
    std::make_pair("single", std::vector<double>{42.0}),
    std::make_pair("duplicates_and_negatives", std::vector<double>{5.0, -1.0, 3.2, 3.2, 0.0}),
    std::make_pair("already_sorted", std::vector<double>{1.0, 2.0, 3.0, 4.0}),
    std::make_pair("reverse_sorted", std::vector<double>{4.0, 3.0, 2.0, 1.0}),
    std::make_pair("mixed", std::vector<double>{9.1, -7.3, 0.0, 5.5, -7.3, 2.2}),
    std::make_pair("odd_size", std::vector<double>{10.0, 3.0, 5.0, 7.0, 2.0, 8.0, 6.0}),
};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<PerepelkinIBatcherOddEvenMergeSortMPI, InType>(kTestParams,
                                                                          PPC_SETTINGS_perepelkin_i_batcher_oddeven_mergesort),
    ppc::util::AddFuncTask<PerepelkinIBatcherOddEvenMergeSortSEQ, InType>(kTestParams,
                                                                          PPC_SETTINGS_perepelkin_i_batcher_oddeven_mergesort));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName =
    PerepelkinIBatcherOddEvenMergeSortFuncTests::PrintFuncTestName<PerepelkinIBatcherOddEvenMergeSortFuncTests>;

INSTANTIATE_TEST_SUITE_P(SortTests, PerepelkinIBatcherOddEvenMergeSortFuncTests, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace perepelkin_i_batcher_oddeven_mergesort
