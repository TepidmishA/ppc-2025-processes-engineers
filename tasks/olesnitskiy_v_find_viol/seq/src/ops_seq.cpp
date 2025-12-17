#include "olesnitskiy_v_find_viol/seq/include/ops_seq.hpp"

#include <vector>

#include "olesnitskiy_v_find_viol/common/include/common.hpp"

namespace olesnitskiy_v_find_viol {

OlesnitskiyVFindViolSEQ::OlesnitskiyVFindViolSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool OlesnitskiyVFindViolSEQ::ValidationImpl() {
  return true;  //! GetInput().empty() проверка на то, что вектор не пустой, но я решил что это не ошибка
}

bool OlesnitskiyVFindViolSEQ::PreProcessingImpl() {
  return true;
}

bool OlesnitskiyVFindViolSEQ::RunImpl() {
  GetOutput() = 0;

  if (GetInput().size() >= 2) {
    const double epsilon = 1e-10;
    for (auto it = GetInput().begin(); it < GetInput().end() - 1;
         it++) {  // использовал auto чтобы без проблем перейти на double
      if (*it - *(it + 1) > epsilon) {
        GetOutput()++;
      }
    }
  }
  return true;
}

bool OlesnitskiyVFindViolSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace olesnitskiy_v_find_viol
