#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace bortsova_a_max_elem_vector {

struct InType {
  std::vector<int> data;
};

using OutType = int;
using TestType = std::tuple<std::vector<int>, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace bortsova_a_max_elem_vector
