#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "olesnitskiy_v_find_viol/common/include/common.hpp"
#include "olesnitskiy_v_find_viol/mpi/include/ops_mpi.hpp"
#include "olesnitskiy_v_find_viol/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace olesnitskiy_v_find_viol {

class OlesnitskiyVFindViolFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<2>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const auto &vector = input_data_;

    int expected_violations = 0;
    for (size_t i = 0; i + 1 < vector.size(); ++i) {
      if (vector[i] > vector[i + 1]) {
        ++expected_violations;
      }
    }

    return output_data == expected_violations;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(OlesnitskiyVFindViolFuncTests, FindViolations) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 10> kTestParam = {
    std::make_tuple(std::vector<double>{}, 0, "empty"),
    std::make_tuple(std::vector<double>{1.0}, 0, "single"),
    std::make_tuple(std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0}, 0, "sorted_asc"),
    std::make_tuple(std::vector<double>{5.0, 4.0, 3.0, 2.0, 1.0}, 4, "sorted_desc"),
    std::make_tuple(std::vector<double>{1.0, 3.0, 2.0, 5.0, 4.0}, 2, "mixed"),
    std::make_tuple(std::vector<double>{1.0, 2.0, 2.0, 3.0, 3.0}, 0, "duplicates"),
    std::make_tuple(std::vector<double>{1.0, 1.0 + 1e-11, 1.0 + 1e-9}, 0, "precision_low"),
    std::make_tuple(std::vector<double>{1.0, 1.0 - 1e-8, 1.0 - 2e-8}, 2, "precision_high"),
    std::make_tuple(std::vector<double>{1.0, 2.0}, 1, "two_numbers"),
    std::make_tuple(std::vector<double>{3.0, 1.0, 2.0}, 1, "three_numbers")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<OlesnitskiyVFindViolMPI, InType>(kTestParam, PPC_SETTINGS_olesnitskiy_v_find_viol),
    ppc::util::AddFuncTask<OlesnitskiyVFindViolSEQ, InType>(kTestParam, PPC_SETTINGS_olesnitskiy_v_find_viol));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = OlesnitskiyVFindViolFuncTests::PrintFuncTestName<OlesnitskiyVFindViolFuncTests>;

INSTANTIATE_TEST_SUITE_P(ViolationTests, OlesnitskiyVFindViolFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace olesnitskiy_v_find_viol
