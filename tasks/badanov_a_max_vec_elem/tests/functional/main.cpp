#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "badanov_a_max_vec_elem/common/include/common.hpp"
#include "badanov_a_max_vec_elem/mpi/include/ops_mpi.hpp"
#include "badanov_a_max_vec_elem/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace badanov_a_max_vec_elem {

class BadanovAMaxVecElemFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param));
  }

 protected:
  void SetUp() override {
    const TestType &params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    test_vector_ = std::get<1>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (test_vector_.empty()) {
      return output_data == INT_MIN;
    }

    int expected_result = test_vector_[0];
    for (size_t i = 1; i < test_vector_.size(); ++i) {
      expected_result = std::max(test_vector_[i], expected_result);
    }
    return output_data == expected_result;
  }

  InType GetTestInputData() final {
    return test_vector_;
  }

 private:
  std::vector<int> test_vector_;
};

namespace {

TEST_P(BadanovAMaxVecElemFuncTests, FunctionalTests) {
  ExecuteTest(GetParam());
}

TEST_P(BadanovAMaxVecElemFuncTests, CoverageTests) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 12> kTestParam = {std::make_tuple(1, std::vector<int>{1, 2, 5, 3, 4, 5}),
                                             std::make_tuple(2, std::vector<int>{-4, -32, -77, -100, -137, -222}),
                                             std::make_tuple(3, std::vector<int>{231, 5363, 3432, 6785, 2345}),
                                             std::make_tuple(4, std::vector<int>{228}),
                                             std::make_tuple(5, std::vector<int>{60, -5, 34, -75, -43, 20}),
                                             std::make_tuple(6, std::vector<int>{99, 99, 99, 99, 99}),
                                             std::make_tuple(7, std::vector<int>{67, 34, 32, 0, -23}),
                                             std::make_tuple(8, std::vector<int>{-56, -7, 45, 56, 304}),
                                             std::make_tuple(9, std::vector<int>{45, 71, 23, 7, 86, 4}),
                                             std::make_tuple(10, std::vector<int>{-23, 8, 29, 56, 87}),
                                             std::make_tuple(11, std::vector<int>{-54, -2, 0, 5, 65, 96}),
                                             std::make_tuple(12, []() {
  std::vector<int> vec(1000);
  for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = static_cast<int>(i + 1);
  }
  vec[999] = 5000;
  return vec;
}())};
const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<BadanovAMaxVecElemMPI, InType>(kTestParam, PPC_SETTINGS_badanov_a_max_vec_elem),
    ppc::util::AddFuncTask<BadanovAMaxVecElemSEQ, InType>(kTestParam, PPC_SETTINGS_badanov_a_max_vec_elem));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = BadanovAMaxVecElemFuncTests::PrintFuncTestName<BadanovAMaxVecElemFuncTests>;

INSTANTIATE_TEST_SUITE_P(VectorMaxTests, BadanovAMaxVecElemFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace badanov_a_max_vec_elem
