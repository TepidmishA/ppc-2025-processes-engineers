#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

#include "bortsova_a_max_elem_vector/common/include/common.hpp"
#include "bortsova_a_max_elem_vector/mpi/include/ops_mpi.hpp"
#include "bortsova_a_max_elem_vector/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace bortsova_a_max_elem_vector {

class BortsovaAMaxElemVectorFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param).size()) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_data_.data = std::get<0>(params);
    if (input_data_.data.empty()) {
      input_data_.data = {0};
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (input_data_.data.empty()) {
      return false;
    }

    int initialized = 0;
    MPI_Initialized(&initialized);
    if (initialized == 0) {
      return ComputeMaxValue(input_data_.data) == output_data;
    }

    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
      return ComputeMaxValue(input_data_.data) == output_data;
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  static int ComputeMaxValue(const std::vector<int> &data) {
    if (data.empty()) {
      return std::numeric_limits<int>::min();
    }
    int current_max = data.front();
    for (std::size_t i = 1; i < data.size(); ++i) {
      current_max = std::max(data[i], current_max);
    }
    return current_max;
  }

  InType input_data_;
};

namespace {

std::vector<int> CreateVector(size_t size, int max_value, size_t max_position) {
  std::vector<int> vec(size);
  for (size_t i = 0; i < size; ++i) {
    const int base = -static_cast<int>(size) + static_cast<int>(i);
    vec[i] = (base * 2) + 3;
  }
  if (max_position < size) {
    vec[max_position] = max_value;
  }
  return vec;
}

TEST_P(BortsovaAMaxElemVectorFuncTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

std::vector<int> CreateAscending(size_t size, int start) {
  std::vector<int> vec(size);
  for (size_t i = 0; i < size; ++i) {
    vec[i] = start + static_cast<int>(i);
  }
  return vec;
}

std::vector<int> CreateDescending(size_t size, int start) {
  std::vector<int> vec(size);
  for (size_t i = 0; i < size; ++i) {
    vec[i] = start - static_cast<int>(i);
  }
  return vec;
}

const std::array<TestType, 42> kTestParam = {

    std::make_tuple(std::vector<int>{42}, "single_positive"),
    std::make_tuple(std::vector<int>{-1000}, "single_negative"),
    std::make_tuple(std::vector<int>{0}, "single_zero"),
    std::make_tuple(std::vector<int>{std::numeric_limits<int>::max()}, "single_max_int"),
    std::make_tuple(std::vector<int>{std::numeric_limits<int>::min()}, "single_min_int"),

    std::make_tuple(std::vector<int>{10, 20, 30, 40}, "four_ascending"),
    std::make_tuple(std::vector<int>{40, 30, 20, 10}, "four_descending"),
    std::make_tuple(std::vector<int>{-5, 100, 50, 75, 25, 150}, "six_mixed"),
    std::make_tuple(std::vector<int>{999, 100, 200, 300, 150, 50, 800, 400}, "eight_random"),
    std::make_tuple(CreateAscending(10, -5), "ten_ascending"),
    std::make_tuple(CreateDescending(10, 100), "ten_descending"),

    std::make_tuple(CreateVector(11, 5555, 5), "size_11"),
    std::make_tuple(CreateVector(13, 7777, 0), "size_13_max_start"),
    std::make_tuple(CreateVector(18, 8888, 17), "size_18_max_end"),
    std::make_tuple(CreateVector(22, 6666, 11), "size_22_max_center"),
    std::make_tuple(CreateVector(27, 9999, 3), "size_27"),
    std::make_tuple(CreateVector(35, 11111, 20), "size_35"),
    std::make_tuple(CreateVector(42, 12345, 21), "size_42"),
    std::make_tuple(CreateVector(55, 15000, 10), "size_55"),
    std::make_tuple(CreateVector(88, 18000, 44), "size_88"),

    std::make_tuple(CreateVector(111, 20000, 55), "size_111"),
    std::make_tuple(CreateVector(222, 25000, 111), "size_222"),
    std::make_tuple(CreateVector(333, 30000, 0), "size_333"),
    std::make_tuple(CreateVector(555, 35000, 277), "size_555"),
    std::make_tuple(CreateVector(777, 40000, 388), "size_777"),
    std::make_tuple(CreateVector(1024, 45000, 512), "size_1024"),
    std::make_tuple(CreateVector(2048, 50000, 1024), "size_2048"),

    std::make_tuple(CreateVector(5000, 55000, 2500), "size_5000"),
    std::make_tuple(CreateVector(15000, 60000, 7500), "size_15000"),
    std::make_tuple(CreateVector(75000, 65000, 37500), "size_75000"),
    std::make_tuple(CreateVector(150000, 70000, 75000), "size_150000"),
    std::make_tuple(CreateVector(500000, 75000, 250000), "size_500000"),
    std::make_tuple(CreateVector(1500000, 80000, 750000), "size_1500000"),

    std::make_tuple(std::vector<int>{std::numeric_limits<int>::min(), -1, 0, 1, std::numeric_limits<int>::max()},
                    "extreme_range"),
    std::make_tuple(std::vector<int>{std::numeric_limits<int>::max() - 1, std::numeric_limits<int>::max(),
                                     std::numeric_limits<int>::max() - 2},
                    "near_max_int"),
    std::make_tuple(std::vector<int>{std::numeric_limits<int>::min(), std::numeric_limits<int>::min() + 1,
                                     std::numeric_limits<int>::min() + 2},
                    "near_min_int"),

    std::make_tuple(std::vector<int>(65, -888888), "all_same_neg_65"),
    std::make_tuple(std::vector<int>(123, 456789), "all_same_pos_123"),
    std::make_tuple(std::vector<int>{-10000, -5000, -100, -50, -1}, "negative_decreasing"),
    std::make_tuple(std::vector<int>{-50000, -40000, -30000, -20000, -10000, -1}, "negative_increasing"),
    std::make_tuple(std::vector<int>{-100, -50, 0, 50, 100, 25, -25}, "symmetric_around_zero")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<BortsovaAMaxElemVectorMpi, InType>(kTestParam, PPC_SETTINGS_bortsova_a_max_elem_vector),
    ppc::util::AddFuncTask<BortsovaAMaxElemVectorSeq, InType>(kTestParam, PPC_SETTINGS_bortsova_a_max_elem_vector));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = BortsovaAMaxElemVectorFuncTests::PrintFuncTestName<BortsovaAMaxElemVectorFuncTests>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, BortsovaAMaxElemVectorFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace bortsova_a_max_elem_vector
