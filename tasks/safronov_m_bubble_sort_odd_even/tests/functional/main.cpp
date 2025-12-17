#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "safronov_m_bubble_sort_odd_even/common/include/common.hpp"
#include "safronov_m_bubble_sort_odd_even/mpi/include/ops_mpi.hpp"
#include "safronov_m_bubble_sort_odd_even/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace safronov_m_bubble_sort_odd_even {

class SafronovMBubbleSortOddEvenFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<0>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<1>(params);
    res_ = std::get<2>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (res_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType res_;
};

namespace {

TEST_P(SafronovMBubbleSortOddEvenFuncTests, BubbleSortOddEvenFunc) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 13> kTestParam = {
    std::make_tuple("a", std::vector<int>{5, 3, 1, 4, 2}, std::vector<int>{1, 2, 3, 4, 5}),
    std::make_tuple("b", std::vector<int>{-3, 10, 0, -1, 5, -2}, std::vector<int>{-3, -2, -1, 0, 5, 10}),
    std::make_tuple("c", std::vector<int>{4, 4, 2, 2, 3, 1, 1}, std::vector<int>{1, 1, 2, 2, 3, 4, 4}),
    std::make_tuple("d", std::vector<int>{1, 2, 3, 5, 4, 6}, std::vector<int>{1, 2, 3, 4, 5, 6}),
    std::make_tuple("e", std::vector<int>{9, 8, 7, 6, 5, 4, 3, 2, 1}, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}),
    std::make_tuple("f", std::vector<int>{1}, std::vector<int>{1}),
    std::make_tuple("g", std::vector<int>{7, 1, 4, 2, 6, 3, 7, 0, 11, 7, 2},
                    std::vector<int>{0, 1, 2, 2, 3, 4, 6, 7, 7, 7, 11}),
    std::make_tuple("h", std::vector<int>{15, -4, 20, -10, 3, 0, 8}, std::vector<int>{-10, -4, 0, 3, 8, 15, 20}),
    std::make_tuple("i", std::vector<int>{}, std::vector<int>{}),
    std::make_tuple("j", std::vector<int>{11, 10}, std::vector<int>{10, 11}),
    std::make_tuple("k", std::vector<int>{12, 5,  9,  0,  -3, 44, 7,  18, 2,  31, 6,  -1, 27, 15, 4,  10, 3,
                                          19, 8,  -5, 14, 13, 1,  22, 17, 29, 11, -2, 33, 28, 16, 20, 26, 21,
                                          24, 30, 32, 23, 25, 34, -4, 35, 36, 37, 38, 39, 40, 41, 42, 43},
                    std::vector<int>{-5, -4, -3, -2, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                     12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
                                     29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44}),
    std::make_tuple(
        "l",
        std::vector<int>{1000, -500, 230, 0,     999, -1,  450, -300, 750,  20,   -700, 33,  18,  600, 55, -200, 888,
                         777,  666,  444, -1000, 321, 123, 222, 111,  -111, -222, -333, 15,  7,   9,   8,  100,  200,
                         300,  -50,  -75, -125,  -20, -10, 500, 501,  502,  -600, 703,  704, 705, 800, -5, -2},
        std::vector<int>{-1000, -700, -600, -500, -333, -300, -222, -200, -125, -111, -75, -50, -20, -10, -5,  -2,  -1,
                         0,     7,    8,    9,    15,   18,   20,   33,   55,   100,  111, 123, 200, 222, 230, 300, 321,
                         444,   450,  500,  501,  502,  600,  666,  703,  704,  705,  750, 777, 800, 888, 999, 1000}),
    std::make_tuple(
        "m", std::vector<int>{3,  -10, 5,  99, -3, -1, 0,  42, 18, -7, 50, 23, -12, 9,  7,  8,   -20, 33, 44, -2,
                              1,  6,   2,  4,  11, 10, -5, -6, 12, 15, 17, 19, -30, 25, 28, -15, 14,  13, 29, -8,
                              21, 22,  -9, 24, 26, 27, 31, 32, 34, 35, 36, 37, 38,  39, 40, 41,  43,  45, 46, 47},
        std::vector<int>{-30, -20, -15, -12, -10, -9, -8, -7, -6, -5, -3, -2, -1, 0,  1,  2,  3,  4,  5,  6,
                         7,   8,   9,   10,  11,  12, 13, 14, 15, 17, 18, 19, 21, 22, 23, 24, 25, 26, 27, 28,
                         29,  31,  32,  33,  34,  35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 50, 99})};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<SafronovMBubbleSortOddEvenMPI, InType>(
                                               kTestParam, PPC_SETTINGS_safronov_m_bubble_sort_odd_even),
                                           ppc::util::AddFuncTask<SafronovMBubbleSortOddEvenSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_safronov_m_bubble_sort_odd_even));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = SafronovMBubbleSortOddEvenFuncTests::PrintFuncTestName<SafronovMBubbleSortOddEvenFuncTests>;

INSTANTIATE_TEST_SUITE_P(BubbleSortOddEvenFunc, SafronovMBubbleSortOddEvenFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace safronov_m_bubble_sort_odd_even
