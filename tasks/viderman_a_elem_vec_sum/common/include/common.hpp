#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace viderman_a_elem_vec_sum {

using InType = std::vector<double>;
using OutType = double;
using TestType = std::tuple<std::string, double>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace viderman_a_elem_vec_sum
