#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "perepelkin_i_qsort_batcher_oddeven_merge/common/include/common.hpp"
#include "perepelkin_i_qsort_batcher_oddeven_merge/mpi/include/ops_mpi.hpp"
#include "perepelkin_i_qsort_batcher_oddeven_merge/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace perepelkin_i_qsort_batcher_oddeven_merge {

class PerepelkinIQsortBatcherOddEvenMergeFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return test_param.first;
  }

 protected:
  void SetUp() override {
    const TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_ = params.second;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (input_data_.empty()) {
      return output_data.empty();
    }

    OutType expected = input_data_;
    std::qsort(expected.data(), expected.size(), sizeof(double), [](const void *a, const void *b) {
      double arg1 = *static_cast<const double *>(a);
      double arg2 = *static_cast<const double *>(b);
      if (arg1 < arg2) {
        return -1;
      }
      if (arg1 > arg2) {
        return 1;
      }
      return 0;
    });
    return expected == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(PerepelkinIQsortBatcherOddEvenMergeFuncTests, SortsCorrectly) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 12> kTestParams = {
    std::make_pair("empty", std::vector<double>{}),
    std::make_pair("single", std::vector<double>{42.0}),
    std::make_pair("duplicates_and_negatives", std::vector<double>{5.0, -1.0, 3.2, 3.2, 0.0}),
    std::make_pair("already_sorted", std::vector<double>{1.0, 2.0, 3.0, 4.0}),
    std::make_pair("reverse_sorted", std::vector<double>{4.0, 3.0, 2.0, 1.0}),
    std::make_pair("mixed", std::vector<double>{9.1, -7.3, 0.0, 5.5, -7.3, 2.2}),
    std::make_pair("odd_size", std::vector<double>{10.0, 3.0, 5.0, 7.0, 2.0, 8.0, 6.0}),
    std::make_pair("even_size", std::vector<double>{8.0, -2.0, 4.0, 9.0, 0.0, -5.0}),
    std::make_pair("many_duplicates", std::vector<double>{1.0, 1.0, 1.0, 2.0, 2.0, 0.0, 0.0}),
    std::make_pair("wide_range", std::vector<double>{1e9, -1e9, 5.5, -12345.6, 9999.9, 0.0}),
    std::make_pair("decimal_precision", std::vector<double>{0.1, 0.1000001, 0.0999999, -0.1, -0.1000001}),
    std::make_pair("longer_random_like",
                   std::vector<double>{12.3, -7.7, 5.5, 0.0, 2.2, 2.2, -3.3, 9.9, -1.1, 4.4, 6.6, -8.8, 7.7}),
};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<PerepelkinIQsortBatcherOddEvenMergeMPI, InType>(
                                               kTestParams, PPC_SETTINGS_perepelkin_i_qsort_batcher_oddeven_merge),
                                           ppc::util::AddFuncTask<PerepelkinIQsortBatcherOddEvenMergeSEQ, InType>(
                                               kTestParams, PPC_SETTINGS_perepelkin_i_qsort_batcher_oddeven_merge));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName =
    PerepelkinIQsortBatcherOddEvenMergeFuncTests::PrintFuncTestName<PerepelkinIQsortBatcherOddEvenMergeFuncTests>;

INSTANTIATE_TEST_SUITE_P(SortTests, PerepelkinIQsortBatcherOddEvenMergeFuncTests, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace perepelkin_i_qsort_batcher_oddeven_merge
