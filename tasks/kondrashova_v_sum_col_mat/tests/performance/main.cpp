#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

#include "kondrashova_v_sum_col_mat/common/include/common.hpp"
#include "kondrashova_v_sum_col_mat/mpi/include/ops_mpi.hpp"
#include "kondrashova_v_sum_col_mat/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace kondrashova_v_sum_col_mat {

class KondrashovaVSumColMatPerfTestsLittle : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  InType input_data;
  OutType expected_output;

  void SetUp() override {
    CreateMatrix(2000, 2000);
  }

  void CreateMatrix(int rows, int cols) {
    int rank = 0;
    int mpi_initialized = 0;

    MPI_Initialized(&mpi_initialized);
    if (mpi_initialized == 1) {
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    }

    if (rank != 0) {
      return;
    }

    input_data.clear();
    expected_output.clear();

    input_data.push_back(rows);
    input_data.push_back(cols);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    std::vector<int> matrix(static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols));
    for (int i = 0; i < rows * cols; i++) {
      matrix[i] = dis(gen);
    }

    input_data.insert(input_data.end(), matrix.begin(), matrix.end());

    expected_output.resize(cols, 0);
    for (int j = 0; j < cols; j++) {
      for (int i = 0; i < rows; i++) {
        expected_output[j] += matrix[(i * cols) + j];
      }
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int rank = 0;
    int mpi_initialized = 0;

    MPI_Initialized(&mpi_initialized);

    if (mpi_initialized == 1) {
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    }

    if (rank != 0) {
      return true;
    }

    if (output_data.empty() && expected_output.empty()) {
      return true;
    }

    if (output_data.size() != expected_output.size()) {
      return false;
    }

    return std::equal(output_data.begin(), output_data.end(), expected_output.begin());
  }

  InType GetTestInputData() final {
    return input_data;
  }
};

class KondrashovaVSumColMatPerfTestsMedium : public KondrashovaVSumColMatPerfTestsLittle {
 protected:
  void SetUp() override {
    CreateMatrix(5000, 5000);
  }
};

class KondrashovaVSumColMatPerfTestsBig : public KondrashovaVSumColMatPerfTestsLittle {
 protected:
  void SetUp() override {
    CreateMatrix(10000, 10000);
  }
};

TEST_P(KondrashovaVSumColMatPerfTestsLittle, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kLittlePerfTasks = ppc::util::MakeAllPerfTasks<InType, KondrashovaVSumColMatMPI, KondrashovaVSumColMatSEQ>(
    PPC_SETTINGS_kondrashova_v_sum_col_mat);

INSTANTIATE_TEST_SUITE_P(LittleMatTest, KondrashovaVSumColMatPerfTestsLittle,
                         ppc::util::TupleToGTestValues(kLittlePerfTasks),
                         KondrashovaVSumColMatPerfTestsLittle::CustomPerfTestName);

TEST_P(KondrashovaVSumColMatPerfTestsMedium, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kMediumPerfTasks = ppc::util::MakeAllPerfTasks<InType, KondrashovaVSumColMatMPI, KondrashovaVSumColMatSEQ>(
    PPC_SETTINGS_kondrashova_v_sum_col_mat);

INSTANTIATE_TEST_SUITE_P(MediumMatTest, KondrashovaVSumColMatPerfTestsMedium,
                         ppc::util::TupleToGTestValues(kMediumPerfTasks),
                         KondrashovaVSumColMatPerfTestsMedium::CustomPerfTestName);

TEST_P(KondrashovaVSumColMatPerfTestsBig, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kBigPerfTasks = ppc::util::MakeAllPerfTasks<InType, KondrashovaVSumColMatMPI, KondrashovaVSumColMatSEQ>(
    PPC_SETTINGS_kondrashova_v_sum_col_mat);

INSTANTIATE_TEST_SUITE_P(BigMatTest, KondrashovaVSumColMatPerfTestsBig, ppc::util::TupleToGTestValues(kBigPerfTasks),
                         KondrashovaVSumColMatPerfTestsBig::CustomPerfTestName);

}  // namespace kondrashova_v_sum_col_mat
