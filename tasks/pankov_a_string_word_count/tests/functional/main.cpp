#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>

#include "pankov_a_string_word_count/common/include/common.hpp"
#include "pankov_a_string_word_count/mpi/include/ops_mpi.hpp"
#include "pankov_a_string_word_count/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace pankov_a_string_word_count {

using LocalTestType = std::tuple<InType, OutType>;

class PankovARunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, LocalTestType> {
 public:
  static std::string PrintTestParam(const LocalTestType &test_param) {
    const auto &input = std::get<0>(test_param);
    const auto &expect = std::get<1>(test_param);
    std::string sanitized_input = ppc::util::test::SanitizeToken(input);
    return sanitized_input + "_expect_" + std::to_string(expect);
  }

 protected:
  void SetUp() override {
    LocalTestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

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
  OutType expected_output_{};
};

namespace {

const std::array<LocalTestType, 8> kTestParam = {
    LocalTestType{InType(""), 0},
    LocalTestType{InType("hello"), 1},
    LocalTestType{InType("hello world"), 2},
    LocalTestType{InType("  many   spaces   here  "), 3},
    LocalTestType{InType("one\ntwo\tthree"), 3},
    LocalTestType{InType("  mix of \n spaces\tand words  "), 5},
    LocalTestType{InType("abc defghijklmnopqrstuvwxyz"), 2},
    LocalTestType{InType("a b c d e f g h i j k l m n o p q r s t u v w x y z"), 26},
};

TEST_P(PankovARunFuncTestsProcesses, StringWordCount) {
  ExecuteTest(GetParam());
}

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<PankovAStringWordCountMPI, InType>(kTestParam, PPC_SETTINGS_pankov_a_string_word_count),
    ppc::util::AddFuncTask<PankovAStringWordCountSEQ, InType>(kTestParam, PPC_SETTINGS_pankov_a_string_word_count));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);
const auto kFuncTestName = PankovARunFuncTestsProcesses::PrintFuncTestName<PankovARunFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(WordCountTests, PankovARunFuncTestsProcesses, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace pankov_a_string_word_count
