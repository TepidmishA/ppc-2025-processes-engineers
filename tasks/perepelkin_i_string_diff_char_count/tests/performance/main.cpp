#include <gtest/gtest.h>

#include "perepelkin_i_string_diff_char_count/common/include/common.hpp"
#include "perepelkin_i_string_diff_char_count/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_string_diff_char_count/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace perepelkin_i_string_diff_char_count {

class PerepelkinIStringDiffCharCountPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
 private:
  InType input_data_ = std::pair("", "");
  OutType expected_count_ = 0;

  void SetUp() override {
    std::string file_name = "performance_large_diff.txt";
    expected_count_ = 12174228;
    
    std::string file_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_perepelkin_i_string_diff_char_count, file_name);
    std::ifstream file(file_path);

    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + file_path);
    }

    std::string str_1, str_2;
    if (!std::getline(file, str_1)) {
      throw std::runtime_error("Failed to read first string from: " + file_path);
    }
    if (!std::getline(file, str_2)) {
      throw std::runtime_error("Failed to read second string from: " + file_path);
    }

    // Fix end of file
    trim_cr(str_1);
    trim_cr(str_2);

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

  static void trim_cr(std::string& s) {
    if (!s.empty() && s.back() == '\r') {
      s.pop_back();
    }
  }
};

TEST_P(PerepelkinIStringDiffCharCountPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, PerepelkinIStringDiffCharCountMPI, PerepelkinIStringDiffCharCountSEQ>(PPC_SETTINGS_perepelkin_i_string_diff_char_count);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = PerepelkinIStringDiffCharCountPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, PerepelkinIStringDiffCharCountPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace perepelkin_i_string_diff_char_count
