#pragma once

#include <string>
#include <tuple>
#include <utility>

#include "task/include/task.hpp"

namespace samoylenko_i_lex_order_check {

using InType = std::pair<std::string, std::string>;
using OutType = bool;
using TestType = std::tuple<std::string, std::string, bool>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace samoylenko_i_lex_order_check
