#pragma once

#include <vector>

#include "safronov_m_bubble_sort_odd_even/common/include/common.hpp"
#include "task/include/task.hpp"

namespace safronov_m_bubble_sort_odd_even {

class SafronovMBubbleSortOddEvenMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit SafronovMBubbleSortOddEvenMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  static std::vector<int> CalculatingInterval(int size_prcs, int rank, int size_arr);
  void SendingVector(int rank);
  static void OddEvenBubble(std::vector<int> &own_data, int own_size, int begin, int phase);
  static void DataExchange(std::vector<int> &own_data, int rank, int neighbor);
  static void BasisSortingLocalArrays(std::vector<int> &own_data, std::vector<int> &interval, int size_arr, int rank,
                                      int size);
  static void EvenPhase(std::vector<int> &own_data, std::vector<int> &interval, int size_arr, int rank, int size);
  static void OddPhase(std::vector<int> &own_data, std::vector<int> &interval, int size_arr, int rank, int size);
  void SendingResult(int rank);
};

}  // namespace safronov_m_bubble_sort_odd_even
