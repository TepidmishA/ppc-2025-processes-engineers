#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <fstream>
#include <string>
#include <tuple>

#include "belov_e_lexico_order_two_strings/common/include/common.hpp"
#include "belov_e_lexico_order_two_strings/mpi/include/ops_mpi.hpp"
#include "belov_e_lexico_order_two_strings/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace belov_e_lexico_order_two_strings {
class BelovERunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    size_t dot = test_param.find('.');
    return test_param.substr(0, dot);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string path = ppc::util::GetAbsoluteTaskPath(PPC_ID_belov_e_lexico_order_two_strings, params);
    std::ifstream file(path);
    std::string line;
    std::getline(file, line);

    size_t first_sep = line.find('_');
    size_t second_sep = line.find('_', first_sep + 1);

    str1_ = line.substr(0, first_sep);
    str2_ = line.substr(first_sep + 1, second_sep - first_sep - 1);
    ans_ = (line.substr(second_sep + 1) == "1");
    file.close();
  }
  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == ans_;
  }

  InType GetTestInputData() final {
    return {str1_, str2_};
  }

 private:
  std::string str1_;
  std::string str2_;
  bool ans_ = false;
};

namespace {
TEST_P(BelovERunFuncTestsProcesses, LexicoOrderTwoStringsFromFiles) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 4> kTestParam = {"test1.txt", "test2.txt", "test3.txt", "test4.txt"};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<BelovELexicoOrderTwoStringsMPI, InType>(
                                               kTestParam, PPC_SETTINGS_belov_e_lexico_order_two_strings),
                                           ppc::util::AddFuncTask<BelovELexicoOrderTwoStringsSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_belov_e_lexico_order_two_strings));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = BelovERunFuncTestsProcesses::PrintFuncTestName<BelovERunFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(LexicoOrderTwoStrings, BelovERunFuncTestsProcesses, kGtestValues, kPerfTestName);

}  // namespace
}  // namespace belov_e_lexico_order_two_strings
