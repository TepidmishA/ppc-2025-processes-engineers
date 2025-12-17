#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

#include "samoylenko_i_lex_order_check/common/include/common.hpp"
#include "samoylenko_i_lex_order_check/mpi/include/ops_mpi.hpp"
#include "samoylenko_i_lex_order_check/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace samoylenko_i_lex_order_check {

class SamoylenkoIRunFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<0>(test_param) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

    input_data_ = std::make_pair(std::get<0>(params), std::get<1>(params));
    expected_ = std::get<2>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == expected_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_{};
};

const std::array<TestType, 10> kTestParam = {
    std::make_tuple("parallel", "sequential", true),
    std::make_tuple("except", "Except", false),
    std::make_tuple("same", "same", true),
    std::make_tuple("abc", "abcd", true),
    std::make_tuple("abcdff", "abcdef", false),
    std::make_tuple("prefix", "fix", false),
    std::make_tuple("post", "postfix", true),
    std::make_tuple("", "hi", true),
    std::make_tuple("bye", "", false),
    std::make_tuple("", "", true),
};

TEST_P(SamoylenkoIRunFuncTests, LexOrderTwoStrings) {
  ExecuteTest(GetParam());
}

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<SamoylenkoILexOrderCheckMPI, InType>(kTestParam, PPC_SETTINGS_samoylenko_i_lex_order_check),
    ppc::util::AddFuncTask<SamoylenkoILexOrderCheckSEQ, InType>(kTestParam, PPC_SETTINGS_samoylenko_i_lex_order_check));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName = SamoylenkoIRunFuncTests::PrintFuncTestName<SamoylenkoIRunFuncTests>;

INSTANTIATE_TEST_SUITE_P(LexOrderTests, SamoylenkoIRunFuncTests, kGtestValues, kFuncTestName);

}  // namespace samoylenko_i_lex_order_check
