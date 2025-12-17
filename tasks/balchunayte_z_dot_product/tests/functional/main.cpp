#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "balchunayte_z_dot_product/common/include/common.hpp"
#include "balchunayte_z_dot_product/mpi/include/ops_mpi.hpp"
#include "balchunayte_z_dot_product/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace balchunayte_z_dot_product {

class BalchunayteZRunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    const int case_id = std::get<0>(params);

    input_data_ = InType{};
    expected_ = 0.0;

    switch (case_id) {
      case 0: {  // simple_positive
        input_data_.a = {1.0, 2.0, 3.0};
        input_data_.b = {4.0, 5.0, 6.0};
        expected_ = 32.0;
        break;
      }
      case 1: {  // negative_values
        input_data_.a = {-1.0, 2.0, -3.0};
        input_data_.b = {4.0, -5.0, 6.0};
        expected_ = -32.0;
        break;
      }
      case 2: {  // zero_vector
        input_data_.a = {0.0, 0.0, 0.0};
        input_data_.b = {1.0, 2.0, 3.0};
        expected_ = 0.0;
        break;
      }
      default:
        input_data_.a = {1.0};
        input_data_.b = {1.0};
        expected_ = 1.0;
        break;
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const double eps = 1e-9;
    return std::fabs(output_data - expected_) < eps;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_{};
  OutType expected_{0.0};
};

namespace {

TEST_P(BalchunayteZRunFuncTestsProcesses, DotProductBasicCases) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 3> kTestParam = {
    std::make_tuple(0, "simple_positive"),
    std::make_tuple(1, "negative_values"),
    std::make_tuple(2, "zero_vector"),
};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<BalchunayteZDotProductMPI, InType>(kTestParam, PPC_SETTINGS_balchunayte_z_dot_product),
    ppc::util::AddFuncTask<BalchunayteZDotProductSEQ, InType>(kTestParam, PPC_SETTINGS_balchunayte_z_dot_product));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName = BalchunayteZRunFuncTestsProcesses::PrintFuncTestName<BalchunayteZRunFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(BalchunayteZDotProductFuncTests, BalchunayteZRunFuncTestsProcesses, kGtestValues,
                         kFuncTestName);

}  // namespace

}  // namespace balchunayte_z_dot_product
