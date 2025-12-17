#include "batkov_f_vector_sum/seq/include/ops_seq.hpp"

#include "batkov_f_vector_sum/common/include/common.hpp"

namespace batkov_f_vector_sum {

BatkovFVectorSumSEQ::BatkovFVectorSumSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool BatkovFVectorSumSEQ::ValidationImpl() {
  return GetOutput() == 0;
}

bool BatkovFVectorSumSEQ::PreProcessingImpl() {
  return true;
}

bool BatkovFVectorSumSEQ::RunImpl() {
  int sum = 0;
  for (int val : GetInput()) {
    sum += val;
  }
  GetOutput() = sum;
  return true;
}

bool BatkovFVectorSumSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace batkov_f_vector_sum
