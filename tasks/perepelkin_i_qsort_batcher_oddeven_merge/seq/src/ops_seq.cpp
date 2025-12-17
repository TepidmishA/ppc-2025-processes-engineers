#include "perepelkin_i_qsort_batcher_oddeven_merge/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <vector>

#include "perepelkin_i_qsort_batcher_oddeven_merge/common/include/common.hpp"

namespace perepelkin_i_qsort_batcher_oddeven_merge {

PerepelkinIQsortBatcherOddEvenMergeSEQ::PerepelkinIQsortBatcherOddEvenMergeSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<double>();
}

bool PerepelkinIQsortBatcherOddEvenMergeSEQ::ValidationImpl() {
  return GetOutput().empty();
}

bool PerepelkinIQsortBatcherOddEvenMergeSEQ::PreProcessingImpl() {
  return true;
}

bool PerepelkinIQsortBatcherOddEvenMergeSEQ::RunImpl() {
  const auto &data = GetInput();
  if (data.empty()) {
    return true;
  }

  std::vector<double> buffer = data;
  std::sort(buffer.begin(), buffer.end());
  GetOutput() = std::move(buffer);
  return true;
}

bool PerepelkinIQsortBatcherOddEvenMergeSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_qsort_batcher_oddeven_merge
