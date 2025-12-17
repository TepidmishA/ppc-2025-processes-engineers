#include "galkin_d_trapezoid_method/seq/include/ops_seq.hpp"

#include "galkin_d_trapezoid_method/common/include/common.hpp"

namespace galkin_d_trapezoid_method {

GalkinDTrapezoidMethodSEQ::GalkinDTrapezoidMethodSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0.0;
}

bool GalkinDTrapezoidMethodSEQ::ValidationImpl() {
  const auto &in = GetInput();
  bool ok_n = in.n > 0;
  bool ok_interval = in.a < in.b;
  bool ok_func = (in.func_id >= 0) && (in.func_id <= 2);
  bool ok_output_init = (GetOutput() == 0.0);

  return ok_n && ok_interval && ok_func && ok_output_init;
}

bool GalkinDTrapezoidMethodSEQ::PreProcessingImpl() {
  GetOutput() = 0.0;
  return true;
}

bool GalkinDTrapezoidMethodSEQ::RunImpl() {
  const auto &in = GetInput();

  const double a = in.a;
  const double b = in.b;
  const int n = in.n;
  const int func_id = in.func_id;

  if (n <= 0 || !(a < b)) {
    return false;
  }

  const double h = (b - a) / static_cast<double>(n);

  double sum = 0.0;
  for (int i = 1; i < n; ++i) {
    const double x = a + (h * static_cast<double>(i));
    sum += Function(x, func_id);
  }

  const double fa = Function(a, func_id);
  const double fb = Function(b, func_id);

  GetOutput() = h * ((fa + fb) * 0.5 + sum);

  return true;
}

bool GalkinDTrapezoidMethodSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace galkin_d_trapezoid_method
