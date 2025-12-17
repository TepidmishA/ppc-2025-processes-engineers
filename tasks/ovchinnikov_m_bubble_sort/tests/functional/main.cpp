#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "ovchinnikov_m_bubble_sort/common/include/common.hpp"
#include "ovchinnikov_m_bubble_sort/mpi/include/ops_mpi.hpp"
#include "ovchinnikov_m_bubble_sort/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace ovchinnikov_m_bubble_sort {

class OvchinnikovMBubbleSortFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    const TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

    input_data_ = std::get<0>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    OutType expected = CalcExpected(input_data_);
    return output_data == expected;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;

  static OutType CalcExpected(const InType &data) {
    OutType sorted = data;
    std::ranges::sort(sorted.begin(), sorted.end());
    return sorted;
  }
};

namespace {

TEST_P(OvchinnikovMBubbleSortFuncTests, BubbleSortTests) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 7> kTestParams = {
    TestType{std::vector<int>{}, "empty"},
    TestType{std::vector<int>{5}, "one_elem"},
    TestType{std::vector<int>{6, 7, 35, 2, 3}, "random_5"},
    TestType{std::vector<int>{1, 2, 3, 4, 5}, "sorted"},
    TestType{std::vector<int>{-1, 100, 0, -5, 20}, "negative"},
    TestType{std::vector<int>{7, 7, 7, 7}, "same_numbers"},
    TestType{std::vector<int>{9, 1, 5, 3, 8, 2, 7, 4, 6, 0}, "random_10"},
};

const auto kTaskList = std::tuple_cat(
    ppc::util::AddFuncTask<OvchinnikovMBubbleSortMPI, InType>(kTestParams, PPC_SETTINGS_ovchinnikov_m_bubble_sort),
    ppc::util::AddFuncTask<OvchinnikovMBubbleSortSEQ, InType>(kTestParams, PPC_SETTINGS_ovchinnikov_m_bubble_sort));

const auto kGtestValues = ppc::util::ExpandToValues(kTaskList);

const auto kTestName = OvchinnikovMBubbleSortFuncTests::PrintFuncTestName<OvchinnikovMBubbleSortFuncTests>;

INSTANTIATE_TEST_SUITE_P(BubbleSortTests, OvchinnikovMBubbleSortFuncTests, kGtestValues, kTestName);

}  // namespace

}  // namespace ovchinnikov_m_bubble_sort
