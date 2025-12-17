#pragma once

#include <mpi.h>

#include <string>
#include <tuple>
#include <vector>

#include "belov_e_lexico_order_two_strings/common/include/common.hpp"
#include "task/include/task.hpp"

namespace belov_e_lexico_order_two_strings {
struct ChunkAns {
  int index;
  int cmp_flag;
};

class BelovELexicoOrderTwoStringsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit BelovELexicoOrderTwoStringsMPI(const InType &in);

 private:
  std::tuple<std::vector<std::string>, std::vector<std::string>> proccesed_input_;
  std::tuple<std::vector<std::string>, std::vector<std::string>> &GetProccesedInput() {
    return proccesed_input_;
  }

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};
void BcastVectorOfStrings(std::vector<std::string> &vec, int n, MPI_Comm comm);
ChunkAns ChunkCheck(const std::vector<std::string> &first, const std::vector<std::string> &second, int begin, int end);
int CeilDiv(int a, int b);
}  // namespace belov_e_lexico_order_two_strings
