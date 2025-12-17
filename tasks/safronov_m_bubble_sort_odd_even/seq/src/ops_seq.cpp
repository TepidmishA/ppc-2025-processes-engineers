#include "safronov_m_bubble_sort_odd_even/seq/include/ops_seq.hpp"

#include <vector>

#include "safronov_m_bubble_sort_odd_even/common/include/common.hpp"

namespace safronov_m_bubble_sort_odd_even {

SafronovMBubbleSortOddEvenSEQ::SafronovMBubbleSortOddEvenSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool SafronovMBubbleSortOddEvenSEQ::ValidationImpl() {
  return GetOutput().empty();
}

bool SafronovMBubbleSortOddEvenSEQ::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool SafronovMBubbleSortOddEvenSEQ::RunImpl() {
  bool flag = true;
  std::vector<int> array = GetInput();
  if (array.empty()) {
    return true;
  }
  int n = static_cast<int>(array.size());
  while (flag) {
    flag = false;
    for (int j = 0; j < n - 1; j += 2) {
      if (array[j] > array[j + 1]) {
        int tmp = array[j + 1];
        array[j + 1] = array[j];
        array[j] = tmp;
        flag = true;
      }
    }
    for (int j = 1; j < n - 1; j += 2) {
      if (array[j] > array[j + 1]) {
        int tmp = array[j + 1];
        array[j + 1] = array[j];
        array[j] = tmp;
        flag = true;
      }
    }
  }
  GetOutput().swap(array);
  return true;
}

bool SafronovMBubbleSortOddEvenSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace safronov_m_bubble_sort_odd_even
