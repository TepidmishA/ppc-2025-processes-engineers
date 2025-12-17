#pragma once
#include <cstdint>
#include <string>
#include <tuple>

#include "matrix.hpp"
#include "task/include/task.hpp"

namespace shakirova_e_elem_matrix_sum {

using InType = Matrix;
using OutType = int64_t;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace shakirova_e_elem_matrix_sum
