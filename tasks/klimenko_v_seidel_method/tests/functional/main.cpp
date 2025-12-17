#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>

#include "klimenko_v_seidel_method/common/include/common.hpp"
#include "klimenko_v_seidel_method/mpi/include/ops_mpi.hpp"
#include "klimenko_v_seidel_method/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace klimenko_v_seidel_method {

class KlimenkoVSeidelMethodFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (input_data_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_ = 0;
};

namespace {

TEST_P(KlimenkoVSeidelMethodFuncTests, FindMatrixMax) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 10> kTestParam = {std::make_tuple(1, "size_1"),   std::make_tuple(2, "size_2"),
                                             std::make_tuple(3, "size_3"),   std::make_tuple(5, "size_5"),
                                             std::make_tuple(7, "size_7"),   std::make_tuple(10, "size_10"),
                                             std::make_tuple(15, "size_15"), std::make_tuple(20, "size_20"),
                                             std::make_tuple(30, "size_30"), std::make_tuple(50, "size_50")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<KlimenkoVSeidelMethodMPI, InType>(kTestParam, PPC_SETTINGS_klimenko_v_seidel_method),
    ppc::util::AddFuncTask<KlimenkoVSeidelMethodSEQ, InType>(kTestParam, PPC_SETTINGS_klimenko_v_seidel_method));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = KlimenkoVSeidelMethodFuncTests::PrintFuncTestName<KlimenkoVSeidelMethodFuncTests>;

INSTANTIATE_TEST_SUITE_P(MatrixFuncTests, KlimenkoVSeidelMethodFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace klimenko_v_seidel_method
