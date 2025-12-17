#include "ovchinnikov_m_bubble_sort/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "ovchinnikov_m_bubble_sort/common/include/common.hpp"

namespace ovchinnikov_m_bubble_sort {

OvchinnikovMBubbleSortSEQ::OvchinnikovMBubbleSortSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  static_cast<void>(GetOutput());
}

bool OvchinnikovMBubbleSortSEQ::ValidationImpl() {
  return true;
}

bool OvchinnikovMBubbleSortSEQ::PreProcessingImpl() {
  return true;
}

bool OvchinnikovMBubbleSortSEQ::RunImpl() {
  auto &array = GetInput();
  if (array.empty()) {
    return true;
  }
  bool sorted = false;
  while (!sorted) {
    sorted = true;
    for (size_t i = 0; i < array.size() - 1; i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }
    for (size_t i = 1; i < array.size() - 1; i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }
  }
  GetOutput() = array;
  return true;
}

bool OvchinnikovMBubbleSortSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace ovchinnikov_m_bubble_sort
