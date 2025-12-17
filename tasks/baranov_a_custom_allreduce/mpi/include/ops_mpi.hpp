#pragma once

#include <mpi.h>

#include <vector>

#include "baranov_a_custom_allreduce/common/include/common.hpp"
#include "task/include/task.hpp"

namespace baranov_a_custom_allreduce {

class BaranovACustomAllreduceMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit BaranovACustomAllreduceMPI(const InType &in);

  static void CustomAllreduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm,
                              int root = 0);
  static void PerformOperation(void *inbuf, void *inoutbuf, int count, MPI_Datatype datatype, MPI_Op op);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static void TreeReduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm,
                         int root);
  static void TreeBroadcast(void *buffer, int count, MPI_Datatype datatype, MPI_Comm comm, int root);

  template <typename T>
  static std::vector<T> GetVectorFromVariant(const InTypeVariant &variant);
};

}  // namespace baranov_a_custom_allreduce
