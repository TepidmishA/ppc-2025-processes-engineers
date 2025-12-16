#include "perepelkin_i_batcher_oddeven_mergesort/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <numeric>
#include <vector>

#include "perepelkin_i_batcher_oddeven_mergesort/common/include/common.hpp"

namespace perepelkin_i_batcher_oddeven_mergesort {

PerepelkinIBatcherOddEvenMergeSortSEQ::PerepelkinIBatcherOddEvenMergeSortSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<double>();
}

bool PerepelkinIBatcherOddEvenMergeSortSEQ::ValidationImpl() {
  return GetOutput().empty();
}

bool PerepelkinIBatcherOddEvenMergeSortSEQ::PreProcessingImpl() {
  return true;
}

bool PerepelkinIBatcherOddEvenMergeSortSEQ::RunImpl() {
  const auto& data = GetInput();
  if (data.empty()) {
    return true;
  }

  const size_t original_size = data.size();
  const size_t padded_size = NextPowerOfTwo(original_size);

  InType buffer = data;
  if (padded_size > original_size) {
    buffer.resize(padded_size, std::numeric_limits<double>::infinity());
  }

  OddEvenMergeSort(buffer, 0, padded_size);
  buffer.resize(original_size);
  GetOutput() = std::move(buffer);
  return true;
}

size_t PerepelkinIBatcherOddEvenMergeSortSEQ::NextPowerOfTwo(const size_t &value) {
  size_t result = 1;
  while (result < value) {
    result <<= 1U;  // Left shift
  }
  return result;
}

void PerepelkinIBatcherOddEvenMergeSortSEQ::OddEvenMergeSort(std::vector<double> &data, const size_t left, const size_t size) {
  if (size <= 1) {
    return;
  }

  const size_t mid = size / 2;
  OddEvenMergeSort(data, left, mid);
  OddEvenMergeSort(data, left + mid, mid);
  OddEvenMerge(data, left, size, 1);
}

void PerepelkinIBatcherOddEvenMergeSortSEQ::OddEvenMerge(std::vector<double> &data, const size_t left, const size_t size, const size_t gap) {
  const size_t step = gap * 2;
  if (step < size) {
    OddEvenMerge(data, left, size, step);
    OddEvenMerge(data, left + gap, size, step);
    for (size_t i = left + gap; i + gap < left + size; i += step) {
      if (data[i] > data[i + gap]) {
        std::swap(data[i], data[i + gap]);
      }
    }
  } else {
    if (data[left] > data[left + gap]) {
      std::swap(data[left], data[left + gap]);
    }
  }
}

bool PerepelkinIBatcherOddEvenMergeSortSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_batcher_oddeven_mergesort
