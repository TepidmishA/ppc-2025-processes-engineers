#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "ermakov_a_numb_viol_elem_vec/common/include/common.hpp"
#include "ermakov_a_numb_viol_elem_vec/mpi/include/ops_mpi.hpp"
#include "ermakov_a_numb_viol_elem_vec/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace ermakov_a_numb_viol_elem_vec {

class ErmakovANumbViolElemVecFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  ErmakovANumbViolElemVecFuncTests() = default;

  static std::string PrintTestParam(const TestType &test_param) {
    const auto &vec = std::get<0>(test_param);
    const int expected = std::get<1>(test_param);

    std::string name = "size_";
    name += std::to_string(vec.size());
    name += "_exp_";
    name += std::to_string(expected);
    return name;
  }

 protected:
  void SetUp() override {
    const TestType &params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(
        ErmakovANumbViolElemVecFuncTests::GetParam());

    input_data_ = std::get<0>(params);
    expected_output_ = std::get<1>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == expected_output_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_{0};
};

const std::array<TestType, 9> kTestParam = {
    std::make_tuple(std::vector<int>{1, 2, 3, 4, 5}, 0),
    std::make_tuple(std::vector<int>{5, 4, 3, 2, 1}, 4),
    std::make_tuple(std::vector<int>{1, 3, 2, 5, 4}, 2),
    std::make_tuple(std::vector<int>{7, 7, 7, 7}, 0),
    std::make_tuple(std::vector<int>{42}, 0),
    std::make_tuple(std::vector<int>{}, 0),
    std::make_tuple(std::vector<int>{1, 3, 2, 4, 3, 5, 4}, 3),
    std::make_tuple(std::vector<int>{2, 1, 3}, 1),
    std::make_tuple(std::vector<int>{1, 2, 1, 3, 2, 4, 3, 5, 4, 6, 5, 7, 6, 8, 7, 9, 8}, 8),
};

namespace {

TEST_P(ErmakovANumbViolElemVecFuncTests, NumbViolElemVec) {
  ExecuteTest(GetParam());
}

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<ErmakovANumbViolElemVecMPI, InType>(kTestParam, PPC_SETTINGS_ermakov_a_numb_viol_elem_vec),
    ppc::util::AddFuncTask<ErmakovANumbViolElemVecSEQ, InType>(kTestParam, PPC_SETTINGS_ermakov_a_numb_viol_elem_vec));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName = ErmakovANumbViolElemVecFuncTests::PrintFuncTestName<ErmakovANumbViolElemVecFuncTests>;

// NOLINTNEXTLINE(cert-err58-cpp, cppcoreguidelines-avoid-non-const-global-variables, modernize-type-traits)
INSTANTIATE_TEST_SUITE_P(NumViolElemVec, ErmakovANumbViolElemVecFuncTests, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace ermakov_a_numb_viol_elem_vec
