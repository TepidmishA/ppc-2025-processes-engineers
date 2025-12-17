#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace olesnitskiy_v_find_viol {

using InType = std::vector<double>;
using OutType = int;
using TestType = std::tuple<std::vector<double>, int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace olesnitskiy_v_find_viol
