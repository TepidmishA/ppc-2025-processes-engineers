#include <gtest/gtest.h>

#include <array>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <tuple>

#include "batkov_f_vector_sum/common/include/common.hpp"
#include "batkov_f_vector_sum/mpi/include/ops_mpi.hpp"
#include "batkov_f_vector_sum/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace batkov_f_vector_sum {

class BatkovFRunFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    std::string filename = std::get<0>(test_param);
    int expected_sum = std::get<1>(test_param);

    std::string sanitized_filename = SanitizeParamName(filename);

    return sanitized_filename + "_" + std::to_string(expected_sum);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string filename = std::get<0>(params);
    expected_sum_ = std::get<1>(params);

    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_batkov_f_vector_sum, filename);
    std::ifstream file(abs_path);

    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + abs_path);
    }

    int value = 0;
    while (file >> value) {
      input_data_.push_back(value);
    }

    file.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_sum_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

  static std::string SanitizeParamName(const std::string &name) {
    std::string sanitized = name;
    for (char &c : sanitized) {
      if (std::isalnum(static_cast<unsigned char>(c)) == 0 && c != '_') {
        c = '_';
      }
    }

    return sanitized;
  }

 private:
  InType input_data_;
  OutType expected_sum_ = 0;
};

namespace {

// NOLINTNEXTLINE
TEST_P(BatkovFRunFuncTestsProcesses, VectorSum) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 4> kTestParam = {
    std::make_tuple("one_hundred_vec.txt", 5050), std::make_tuple("two_hundred_vec.txt", 2900),
    std::make_tuple("odd_size_vector.txt", 1401), std::make_tuple("empty_vec.txt", 0)};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<BatkovFVectorSumMPI, InType>(kTestParam, PPC_SETTINGS_batkov_f_vector_sum),
                   ppc::util::AddFuncTask<BatkovFVectorSumSEQ, InType>(kTestParam, PPC_SETTINGS_batkov_f_vector_sum));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = BatkovFRunFuncTestsProcesses::PrintFuncTestName<BatkovFRunFuncTestsProcesses>;

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(VectorSumFuncTests, BatkovFRunFuncTestsProcesses, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace batkov_f_vector_sum
