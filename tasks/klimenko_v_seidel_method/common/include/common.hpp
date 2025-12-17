#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace klimenko_v_seidel_method {

using InType = int;
using OutType = int;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace klimenko_v_seidel_method
