#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace belov_e_lexico_order_two_strings {

using InType = std::tuple<std::string, std::string>;
using OutType = bool;
using TestType = std::string;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace belov_e_lexico_order_two_strings
