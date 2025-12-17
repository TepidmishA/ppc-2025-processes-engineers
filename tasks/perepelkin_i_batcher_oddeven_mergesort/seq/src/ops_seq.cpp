#include "perepelkin_i_batcher_oddeven_mergesort/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
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
  const auto &data = GetInput();
  if (data.empty()) {
    return true;
  }

  std::vector<double> buffer = data;
  std::sort(buffer.begin(), buffer.end());
  GetOutput() = std::move(buffer);
  return true;
}

bool PerepelkinIBatcherOddEvenMergeSortSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_batcher_oddeven_mergesort
