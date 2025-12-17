#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

#include "posternak_a_count_different_char_in_two_lines/common/include/common.hpp"
#include "posternak_a_count_different_char_in_two_lines/mpi/include/ops_mpi.hpp"
#include "posternak_a_count_different_char_in_two_lines/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace posternak_a_count_different_char_in_two_lines {

class PosternakACountDifferentCharInTwoLinesFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int test = std::get<0>(params);

    switch (test) {
      case 1:
        input_data_ = std::make_pair("string", "strong");
        expected_output_ = 1;
        break;
      case 2:
        input_data_ = std::make_pair("stringstring", "string");
        expected_output_ = 6;
        break;
      case 3:
        input_data_ = std::make_pair("string", "stringstring");
        expected_output_ = 6;
        break;
      case 4:
        input_data_ = std::make_pair("string", "string");
        expected_output_ = 0;
        break;
      case 5:
        input_data_ = std::make_pair("string", "STRING");
        expected_output_ = 6;
        break;
      case 6:
        input_data_ = std::make_pair("x", "string");
        expected_output_ = 6;
        break;
      case 7:
        input_data_ = std::make_pair("string", "x");
        expected_output_ = 6;
        break;
      case 8:
        input_data_ = std::make_pair("s", "g");
        expected_output_ = 1;
        break;
      case 9:
        input_data_ = std::make_pair("s", "s");
        expected_output_ = 0;
        break;
      case 10:
        input_data_ = std::make_pair("-@$%#", "_@%$#");
        expected_output_ = 3;
        break;
      case 11:
        input_data_ = std::make_pair("strong string", "strong_string");
        expected_output_ = 1;
        break;
      case 12:
        input_data_ = std::make_pair("12345", "54321");
        expected_output_ = 4;
        break;
      case 13:
        input_data_ = std::make_pair("12345", "12345");
        expected_output_ = 0;
        break;
      case 14:
        input_data_ = std::make_pair("7826", "7828");
        expected_output_ = 1;
        break;
      case 15:
        input_data_ = std::make_pair("s t r i n g", "string");
        expected_output_ = 10;
        break;
      case 16:
        input_data_ = std::make_pair("1wertyuiopasdfghjkl", "qwertyuiopasdfghjkl");
        expected_output_ = 1;
        break;
      case 17:
        input_data_ = std::make_pair("qwertyuiopasdfghjkl", "qwertyuiopasdfghjkk");
        expected_output_ = 1;
        break;
      case 18:
        input_data_ = std::make_pair("qwertyuiopasdfghjkl", "qwertyuiopasdfghjkl");
        expected_output_ = 0;
        break;
      case 19:
        input_data_ = std::make_pair("str\ning", "string");
        expected_output_ = 4;
        break;
      case 20:
        input_data_ = std::make_pair("strong\tstring", "strong string");
        expected_output_ = 1;
        break;
      default:
        break;
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_output_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_{};
};

namespace {

TEST_P(PosternakACountDifferentCharInTwoLinesFuncTests, CountDifferentCharInTwoLines) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 20> kTestParam = {std::make_tuple(1, "one_diff_char"),
                                             std::make_tuple(2, "s1_len_bigger_s2_len"),
                                             std::make_tuple(3, "s1_len_less_s2_len"),
                                             std::make_tuple(4, "equal_strings"),
                                             std::make_tuple(5, "diff_reg"),
                                             std::make_tuple(6, "s1_one_char"),
                                             std::make_tuple(7, "s2_one_char"),
                                             std::make_tuple(8, "diff_one_char_strings"),
                                             std::make_tuple(9, "equal_one_char_strings"),
                                             std::make_tuple(10, "special_chars"),
                                             std::make_tuple(11, "string_sith_space"),
                                             std::make_tuple(12, "nums_reverse"),
                                             std::make_tuple(13, "equal_nums"),
                                             std::make_tuple(14, "one_diff_num"),
                                             std::make_tuple(15, "many_spaces"),
                                             std::make_tuple(16, "long_strings_diff_begin"),
                                             std::make_tuple(17, "long_strings_diff_end"),
                                             std::make_tuple(18, "equal_long_strings"),
                                             std::make_tuple(19, "enter"),
                                             std::make_tuple(20, "tab")};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<PosternakACountDifferentCharInTwoLinesMPI, InType>(
                                               kTestParam, PPC_SETTINGS_posternak_a_count_different_char_in_two_lines),
                                           ppc::util::AddFuncTask<PosternakACountDifferentCharInTwoLinesSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_posternak_a_count_different_char_in_two_lines));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    PosternakACountDifferentCharInTwoLinesFuncTests::PrintFuncTestName<PosternakACountDifferentCharInTwoLinesFuncTests>;

INSTANTIATE_TEST_SUITE_P(StringTests, PosternakACountDifferentCharInTwoLinesFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace posternak_a_count_different_char_in_two_lines
