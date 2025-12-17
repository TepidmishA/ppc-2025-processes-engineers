#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace galkin_d_trapezoid_method {

struct Input {
  double a;     // левая граница интегрирования
  double b;     // правая граница
  int n;        // число разбиений (трапеций)
  int func_id;  // id функции для интегрирования
};

using InType = Input;
using OutType = double;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

enum class FunctionId : std::uint8_t {
  kLinear = 0,     // f(x) = x
  kQuadratic = 1,  // f(x) = x^2
  kSin = 2         // f(x) = sin(x)
};

inline bool IsValidFunctionId(int func_id) {
  return func_id >= static_cast<int>(FunctionId::kLinear) && func_id <= static_cast<int>(FunctionId::kSin);
}

inline double Function(double x, int func_id) {
  if (!IsValidFunctionId(func_id)) {
    return 0.0;
  }

  const auto id = static_cast<FunctionId>(func_id);
  switch (id) {
    case FunctionId::kLinear:
      return x;  // f(x) = x
    case FunctionId::kQuadratic:
      return x * x;  // f(x) = x^2
    case FunctionId::kSin:
      return std::sin(x);  // f(x) = sin(x)
    default:
      return 0.0;
  }
}

inline double GetExactIntegral(const InType &in) {
  if (!IsValidFunctionId(in.func_id)) {
    return 0.0;
  }

  const double a = in.a;
  const double b = in.b;
  const auto id = static_cast<FunctionId>(in.func_id);

  switch (id) {
    case FunctionId::kLinear:
      return (b * b - a * a) / 2.0;
    case FunctionId::kQuadratic:
      return (b * b * b - a * a * a) / 3.0;
    case FunctionId::kSin:
      return std::cos(a) - std::cos(b);
    default:
      return 0.0;
  }
}

}  // namespace galkin_d_trapezoid_method
