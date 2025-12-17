#include <gtest/gtest.h>

#include <array>
#include <cctype>
#include <cstddef>
#include <string>
#include <tuple>

#include "borunov_v_cnt_words/common/include/common.hpp"
#include "borunov_v_cnt_words/mpi/include/ops_mpi.hpp"
#include "borunov_v_cnt_words/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace borunov_v_cnt_words {

class BorunovVCntWordsFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    std::string input_str = std::get<0>(test_param);

    if (input_str.length() > 20) {
      input_str = input_str.substr(0, 20);
    }

    std::string sanitized_input;
    for (char c : input_str) {
      if (std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_') {
        sanitized_input += c;
      } else {
        sanitized_input += '_';
      }
    }

    std::string expected_count = std::to_string(std::get<1>(test_param));

    return "Input_" + sanitized_input + "_Expected_" + expected_count;
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = std::get<0>(params);
    expected_output_ = std::get<1>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_output_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_ = 0;
};

namespace {

TEST_P(BorunovVCntWordsFuncTests, WordCountVerification) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 8> kTestParam = {std::make_tuple("Hello world", 2),
                                            std::make_tuple("This is a test", 4),
                                            std::make_tuple("One two  three   four", 4),
                                            std::make_tuple("word", 1),
                                            std::make_tuple("    spaced    out     words     ", 3),
                                            std::make_tuple("\nOne\ttwo\three\nfour", 4),
                                            std::make_tuple("  ", 0)};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<BorunovVCntWordsMPI, InType>(kTestParam, PPC_SETTINGS_borunov_v_cnt_words),
                   ppc::util::AddFuncTask<BorunovVCntWordsSEQ, InType>(kTestParam, PPC_SETTINGS_borunov_v_cnt_words));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = BorunovVCntWordsFuncTests::PrintFuncTestName<BorunovVCntWordsFuncTests>;

INSTANTIATE_TEST_SUITE_P(WordCountTests, BorunovVCntWordsFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace borunov_v_cnt_words
