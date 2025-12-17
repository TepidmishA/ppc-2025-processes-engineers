#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "kondrashova_v_sum_col_mat/common/include/common.hpp"
#include "kondrashova_v_sum_col_mat/mpi/include/ops_mpi.hpp"
#include "kondrashova_v_sum_col_mat/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace kondrashova_v_sum_col_mat {

class KondrashovaVSumColMatFuncTestsProcesses : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

    int file_index = std::get<0>(params);
    std::string filename = "test" + std::to_string(file_index) + ".txt";

    std::vector<std::string> possible_paths = {
        ppc::util::GetAbsoluteTaskPath(PPC_ID_kondrashova_v_sum_col_mat, "data/" + filename),
        ppc::util::GetAbsoluteTaskPath(PPC_ID_kondrashova_v_sum_col_mat, filename),
        ppc::util::GetAbsoluteTaskPath(PPC_ID_kondrashova_v_sum_col_mat, "tests/data/" + filename),
    };

    std::ifstream file;
    bool found = false;

    for (const auto &path : possible_paths) {
      file.open(path);
      if (file.is_open()) {
        found = true;
        break;
      }
    }

    if (!found) {
      throw std::runtime_error("Failed to open matrix file");
    }

    int rows = 0;
    int cols = 0;
    if (!(file >> rows >> cols)) {
      file.close();
      throw std::runtime_error("Failed to read matrix dimensions");
    }

    if (rows <= 0 || cols <= 0) {
      file.close();
      throw std::runtime_error("Invalid matrix dimensions");
    }

    input_data_.clear();
    input_data_.push_back(rows);
    input_data_.push_back(cols);

    const std::size_t total_size =
        static_cast<std::size_t>(2) + (static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols));

    input_data_.reserve(total_size);

    expected_output_.clear();
    expected_output_.resize(static_cast<std::size_t>(cols), 0);

    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < cols; ++j) {
        int val = 0;
        if (!(file >> val)) {
          file.close();
          throw std::runtime_error("Failed to read matrix element");
        }
        input_data_.push_back(val);
        expected_output_[static_cast<std::size_t>(j)] += val;
      }
    }

    file.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int rank = 0;
    int mpi_initialized = 0;

    MPI_Initialized(&mpi_initialized);
    if (mpi_initialized != 0) {
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    }

    if (rank != 0) {
      return true;
    }

    if (output_data.empty() && expected_output_.empty()) {
      return true;
    }

    if (output_data.size() != expected_output_.size()) {
      return false;
    }

    return std::equal(output_data.begin(), output_data.end(), expected_output_.begin());
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_;
};

namespace {

TEST_P(KondrashovaVSumColMatFuncTestsProcesses, SumColumnsMatrix) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 5> kTestParam = {std::make_tuple(1, "test1"), std::make_tuple(2, "test2"),
                                            std::make_tuple(3, "test3"), std::make_tuple(4, "test4"),
                                            std::make_tuple(5, "test5")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<KondrashovaVSumColMatMPI, InType>(kTestParam, PPC_SETTINGS_kondrashova_v_sum_col_mat),
    ppc::util::AddFuncTask<KondrashovaVSumColMatSEQ, InType>(kTestParam, PPC_SETTINGS_kondrashova_v_sum_col_mat));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kTestNameGenerator =
    KondrashovaVSumColMatFuncTestsProcesses::PrintFuncTestName<KondrashovaVSumColMatFuncTestsProcesses>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests_KondrashovaVSumColMatProcesses, KondrashovaVSumColMatFuncTestsProcesses,
                         kGtestValues, kTestNameGenerator);

}  // namespace

}  // namespace kondrashova_v_sum_col_mat
