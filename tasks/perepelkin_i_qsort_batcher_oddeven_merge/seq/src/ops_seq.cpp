#include "perepelkin_i_qsort_batcher_oddeven_merge/seq/include/ops_seq.hpp"

#include <cstdlib>
#include <utility>
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
  std::qsort(buffer.data(), buffer.size(), sizeof(double), [](const void *a, const void *b) {
    double arg1 = *static_cast<const double *>(a);
    double arg2 = *static_cast<const double *>(b);
    if (arg1 < arg2) {
      return -1;
    }
    if (arg1 > arg2) {
      return 1;
    }
    return 0;
  });

  GetOutput() = std::move(buffer);
  return true;
}

bool PerepelkinIQsortBatcherOddEvenMergeSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace perepelkin_i_qsort_batcher_oddeven_merge
