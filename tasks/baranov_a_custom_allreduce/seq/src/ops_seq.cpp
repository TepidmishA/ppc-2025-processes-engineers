#include "baranov_a_custom_allreduce/seq/include/ops_seq.hpp"

#include <cmath>
#include <exception>
#include <variant>

#include "baranov_a_custom_allreduce/common/include/common.hpp"

namespace baranov_a_custom_allreduce {

BaranovACustomAllreduceSEQ::BaranovACustomAllreduceSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = in;
}

bool BaranovACustomAllreduceSEQ::ValidationImpl() {
  try {
    return true;
  } catch (...) {
    return false;
  }
}

bool BaranovACustomAllreduceSEQ::PreProcessingImpl() {
  return true;
}

bool BaranovACustomAllreduceSEQ::RunImpl() {
  try {
    GetOutput() = GetInput();
    return true;
  } catch (const std::exception &) {
    return false;
  }
}

bool BaranovACustomAllreduceSEQ::PostProcessingImpl() {
  return true;
}
}  // namespace baranov_a_custom_allreduce
