#include <gtest/gtest.h>

#include <cstddef>
#include <random>
#include <string>
#include <tuple>
#include <utility>

#include "perepelkin_i_string_diff_char_count/common/include/common.hpp"
#include "perepelkin_i_string_diff_char_count/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_string_diff_char_count/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace perepelkin_i_string_diff_char_count {

class PerepelkinIStringDiffCharCountPerfTestProcesses : public ppc::util::BaseRunPerfTests<InType, OutType> {
 private:
  InType input_data_ = std::pair("", "");
  OutType expected_count_ = 0;

  size_t base_length_ = 1000000;
  int scale_factor_ = 512;
  unsigned int seed_ = 29;
  float diff_rate_ = 0.1F;

  void SetUp() override {
    auto [str1, str2, diff_count] = GenerateTestData(base_length_, seed_, diff_rate_, scale_factor_);
    input_data_ = std::make_pair(str1, str2);
    expected_count_ = diff_count;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_count_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

  static std::tuple<std::string, std::string, int> GenerateTestData(size_t base_length, unsigned int seed,
                                                                    float diff_rate = 0.15F, int scale_factor = 1) {
    std::mt19937 gen(seed);

    std::string base_str1;
    std::string base_str2;
    base_str1.reserve(base_length);
    base_str2.reserve(base_length);

    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::uniform_int_distribution<size_t> char_dist(0, charset.size() - 1);
    std::uniform_real_distribution<double> diff_dist(0.0, 1.0);

    int base_diff_count = 0;

    // Generate base pattern
    for (size_t i = 0; i < base_length; ++i) {
      char c1 = charset[char_dist(gen)];
      base_str1.push_back(c1);

      if (diff_dist(gen) < diff_rate) {
        char c2 = charset[char_dist(gen)];
        while (c2 == c1) {
          c2 = charset[char_dist(gen)];
        }
        base_str2.push_back(c2);
        base_diff_count++;
      } else {
        base_str2.push_back(c1);
      }
    }

    // Scale strings by repeating the base pattern
    std::string str1;
    std::string str2;
    size_t scaled_length = base_length * scale_factor;
    str1.reserve(scaled_length);
    str2.reserve(scaled_length);

    for (int i = 0; i < scale_factor; ++i) {
      str1 += base_str1;
      str2 += base_str2;
    }
    int total_diff_count = base_diff_count * scale_factor;

    // Extend first string to be longer
    size_t extension_length = scaled_length / 1000;  // 0.1% extension
    for (size_t i = 0; i < extension_length; ++i) {
      str1.push_back(charset[char_dist(gen)]);
    }
    total_diff_count += static_cast<int>(extension_length);

    return {str1, str2, total_diff_count};
  }
};

TEST_P(PerepelkinIStringDiffCharCountPerfTestProcesses, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, PerepelkinIStringDiffCharCountMPI, PerepelkinIStringDiffCharCountSEQ>(
        PPC_SETTINGS_perepelkin_i_string_diff_char_count);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = PerepelkinIStringDiffCharCountPerfTestProcesses::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, PerepelkinIStringDiffCharCountPerfTestProcesses, kGtestValues, kPerfTestName);

}  // namespace perepelkin_i_string_diff_char_count
