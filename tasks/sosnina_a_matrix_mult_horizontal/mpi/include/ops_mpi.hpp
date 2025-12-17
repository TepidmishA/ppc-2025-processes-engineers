#pragma once

#include <vector>

#include "sosnina_a_matrix_mult_horizontal/common/include/common.hpp"
#include "task/include/task.hpp"

namespace sosnina_a_matrix_mult_horizontal {

class SosninaAMatrixMultHorizontalMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }

  explicit SosninaAMatrixMultHorizontalMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  bool RunSequential();

  bool PrepareAndValidateSizes(int &rows_a, int &cols_a, int &rows_b, int &cols_b);
  void PrepareAndBroadcastMatrixB(std::vector<double> &b_flat, int rows_b, int cols_b);
  void DistributeMatrixAData(std::vector<int> &my_row_indices, std::vector<double> &local_a_flat, int &local_rows,
                             int rows_a, int cols_a);
  static void ComputeLocalMultiplication(const std::vector<double> &local_a_flat, const std::vector<double> &b_flat,
                                         std::vector<double> &local_result_flat, int local_rows, int cols_a,
                                         int cols_b);
  void GatherResults(std::vector<double> &final_result_flat, const std::vector<int> &my_row_indices,
                     const std::vector<double> &local_result_flat, int local_rows, int rows_a, int cols_b) const;

  void FillLocalAFlat(const std::vector<int> &my_row_indices, std::vector<double> &local_a_flat, int cols_a);
  void SendRowsToProcess(int dest, const std::vector<int> &dest_rows, int cols_a);
  [[nodiscard]] std::vector<int> GetRowsForProcess(int process_rank, int rows_a) const;
  static void ReceiveRowsFromRoot(int &local_rows, std::vector<int> &my_row_indices, std::vector<double> &local_a_flat,
                                  int cols_a);

  static void CollectLocalResults(const std::vector<int> &my_row_indices, const std::vector<double> &local_result_flat,
                                  std::vector<double> &final_result_flat, int cols_b);
  void ReceiveResultsFromProcess(int src, std::vector<double> &final_result_flat, int cols_b) const;
  static void SendLocalResults(const std::vector<double> &local_result_flat, int local_rows, int cols_b);

  void ConvertToMatrix(const std::vector<double> &final_result_flat, int rows_a, int cols_b);

  std::vector<std::vector<double>> matrix_A_;
  std::vector<std::vector<double>> matrix_B_;
  std::vector<std::vector<double>> result_matrix_;
  int rank_ = 0;
  int world_size_ = 1;
};

}  // namespace sosnina_a_matrix_mult_horizontal
