#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include "perepelkin_i_string_diff_char_count/common/include/common.hpp"
#include "perepelkin_i_string_diff_char_count/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_string_diff_char_count/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace perepelkin_i_string_diff_char_count {

class PerepelkinIStringDiffCharCountFuncTestProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return FormatFileName(test_param.first) + "_" + std::to_string(test_param.second);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string file_name = params.first;
    expected_count_ = params.second;

    std::string file_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_perepelkin_i_string_diff_char_count, file_name);
    std::ifstream file(file_path);

    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + file_path);
    }

    std::string str_1;
    std::string str_2;
    if (!std::getline(file, str_1)) {
      throw std::runtime_error("Failed to read first string from: " + file_path);
    }
    if (!std::getline(file, str_2)) {
      throw std::runtime_error("Failed to read second string from: " + file_path);
    }

    // Fix for end of file
    TrimCr(str_1);
    TrimCr(str_2);

    std::string extra_line;
    if (std::getline(file, extra_line) && !extra_line.empty()) {
      throw std::runtime_error("Unexpected extra data in: " + file_path + " (expected only two strings)");
    }

    input_data_ = std::make_pair(str_1, str_2);
    file.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_count_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_ = std::pair("", "");
  OutType expected_count_ = 0;

  static std::string FormatFileName(const std::string &filename) {
    size_t dot_index = filename.find_last_of('.');
    if (dot_index != std::string::npos) {
      return filename.substr(0, dot_index);
    }
    return filename;
  }

  static void TrimCr(std::string &s) {
    if (!s.empty() && s.back() == '\r') {
      s.pop_back();
    }
  }
};

namespace {

TEST_P(PerepelkinIStringDiffCharCountFuncTestProcesses, StringDifFromFile) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 12> kTestParam = {
    std::make_pair("test_first_empty.txt", 6),          std::make_pair("test_second_empty.txt", 6),
    std::make_pair("test_empty_strings.txt", 0),        std::make_pair("test_identical_short.txt", 0),
    std::make_pair("test_single_diff.txt", 1),          std::make_pair("test_diff_length_extra_chars.txt", 1),
    std::make_pair("test_completely_different.txt", 4), std::make_pair("test_with_spaces.txt", 2),
    std::make_pair("test_case_sensitive.txt", 1),       std::make_pair("test_long_strings_partial_diff.txt", 1),
    std::make_pair("test_long_diff_tail.txt", 3),       std::make_pair("test_special_chars.txt", 1)};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<PerepelkinIStringDiffCharCountMPI, InType>(
                                               kTestParam, PPC_SETTINGS_perepelkin_i_string_diff_char_count),
                                           ppc::util::AddFuncTask<PerepelkinIStringDiffCharCountSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_perepelkin_i_string_diff_char_count));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName =
    PerepelkinIStringDiffCharCountFuncTestProcesses::PrintFuncTestName<PerepelkinIStringDiffCharCountFuncTestProcesses>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, PerepelkinIStringDiffCharCountFuncTestProcesses, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace perepelkin_i_string_diff_char_count
