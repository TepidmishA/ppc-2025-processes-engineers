#include "sinev_a_min_in_vector/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

#include "sinev_a_min_in_vector/common/include/common.hpp"

namespace sinev_a_min_in_vector {

SinevAMinInVectorSEQ::SinevAMinInVectorSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::numeric_limits<int>::max();
}

bool SinevAMinInVectorSEQ::ValidationImpl() {
  return !GetInput().empty();
}

bool SinevAMinInVectorSEQ::PreProcessingImpl() {
  GetOutput() = std::numeric_limits<int>::max();
  return true;
}

bool SinevAMinInVectorSEQ::RunImpl() {
  if (GetInput().empty()) {
    return true;
  }

  GetOutput() = GetInput()[0];

  for (size_t i = 1; i < GetInput().size(); i++) {
    GetOutput() = std::min(GetInput()[i], GetOutput());
  }

  return true;
}

bool SinevAMinInVectorSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace sinev_a_min_in_vector
