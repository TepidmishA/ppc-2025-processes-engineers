#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace balchunayte_z_dot_product {

struct InType {
  std::vector<double> a;
  std::vector<double> b;
};

using OutType = double;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace balchunayte_z_dot_product
