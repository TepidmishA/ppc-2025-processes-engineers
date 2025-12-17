#include "balchunayte_z_dot_product/seq/include/ops_seq.hpp"

#include <cstddef>

#include "balchunayte_z_dot_product/common/include/common.hpp"

namespace balchunayte_z_dot_product {

BalchunayteZDotProductSEQ::BalchunayteZDotProductSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool BalchunayteZDotProductSEQ::ValidationImpl() {
  const auto &in = GetInput();
  const auto &a = in.a;
  const auto &b = in.b;

  if (a.empty() || b.empty()) {
    return false;
  }
  if (a.size() != b.size()) {
    return false;
  }
  return true;
}

bool BalchunayteZDotProductSEQ::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

bool BalchunayteZDotProductSEQ::RunImpl() {
  const auto &in = GetInput();
  const auto &a = in.a;
  const auto &b = in.b;

  double sum = 0.0;
  for (std::size_t i = 0; i < a.size(); ++i) {
    sum += a[i] * b[i];
  }

  GetOutput() = sum;
  return true;
}

bool BalchunayteZDotProductSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace balchunayte_z_dot_product
