#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace batkov_f_vector_sum {

using InType = std::vector<int>;
using OutType = int;
using TestType = std::tuple<std::string, int>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace batkov_f_vector_sum
