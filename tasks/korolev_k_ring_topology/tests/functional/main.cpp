#include <gtest/gtest.h>
#include <mpi.h>

#include <cstddef>
#include <vector>

#include "korolev_k_ring_topology/common/include/common.hpp"
#include "korolev_k_ring_topology/mpi/include/ops_mpi.hpp"
#include "korolev_k_ring_topology/seq/include/ops_seq.hpp"

namespace korolev_k_ring_topology {

class KorolevKRingTopologyFuncTest : public ::testing::Test {
 protected:
  static int GetWorldSize() {
    int size = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    return size;
  }

  static int GetWorldRank() {
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank;
  }
};

// Тест 1: Передача от процесса 0 к процессу 1 (соседи)
TEST_F(KorolevKRingTopologyFuncTest, SendFromZeroToOne) {
  int size = GetWorldSize();
  if (size < 2) {
    GTEST_SKIP() << "Need at least 2 processes";
  }

  RingMessage input;
  input.source = 0;
  input.dest = 1;
  input.data = {1, 2, 3, 4, 5};

  KorolevKRingTopologyMPI task(input);

  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());

  auto output = task.GetOutput();
  EXPECT_EQ(output, input.data);
}

// Тест 2: Передача от последнего процесса к процессу 0 (через кольцо)
TEST_F(KorolevKRingTopologyFuncTest, SendFromLastToZero) {
  int size = GetWorldSize();
  if (size < 2) {
    GTEST_SKIP() << "Need at least 2 processes";
  }

  RingMessage input;
  input.source = size - 1;
  input.dest = 0;
  input.data = {10, 20, 30};

  KorolevKRingTopologyMPI task(input);

  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());

  auto output = task.GetOutput();
  EXPECT_EQ(output, input.data);
}

// Тест 3: Передача самому себе (source == dest)
TEST_F(KorolevKRingTopologyFuncTest, SendToSelf) {
  RingMessage input;
  input.source = 0;
  input.dest = 0;
  input.data = {100, 200, 300, 400};

  KorolevKRingTopologyMPI task(input);

  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());

  auto output = task.GetOutput();
  EXPECT_EQ(output, input.data);
}

// Тест 4: Передача через несколько процессов
TEST_F(KorolevKRingTopologyFuncTest, SendThroughMultipleProcesses) {
  int size = GetWorldSize();
  if (size < 4) {
    GTEST_SKIP() << "Need at least 4 processes";
  }

  RingMessage input;
  input.source = 0;
  input.dest = size - 1;
  input.data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  KorolevKRingTopologyMPI task(input);

  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());

  auto output = task.GetOutput();
  EXPECT_EQ(output, input.data);
}

// Тест 5: Передача пустого массива
TEST_F(KorolevKRingTopologyFuncTest, SendEmptyData) {
  int size = GetWorldSize();
  if (size < 2) {
    GTEST_SKIP() << "Need at least 2 processes";
  }

  RingMessage input;
  input.source = 0;
  input.dest = 1;
  input.data = {};

  KorolevKRingTopologyMPI task(input);

  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());

  auto output = task.GetOutput();
  EXPECT_TRUE(output.empty());
}

// Тест 6: Передача большого массива данных
TEST_F(KorolevKRingTopologyFuncTest, SendLargeData) {
  int size = GetWorldSize();
  if (size < 2) {
    GTEST_SKIP() << "Need at least 2 processes";
  }

  RingMessage input;
  input.source = 0;
  input.dest = size / 2;
  input.data.resize(1000);
  for (int i = 0; i < 1000; ++i) {
    input.data[static_cast<std::size_t>(i)] = i * 2;
  }

  KorolevKRingTopologyMPI task(input);

  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());

  auto output = task.GetOutput();
  EXPECT_EQ(output, input.data);
}

// Тест 7: SEQ версия - простое копирование
TEST_F(KorolevKRingTopologyFuncTest, SeqCopyData) {
  RingMessage input;
  input.source = 0;
  input.dest = 1;
  input.data = {5, 10, 15, 20};

  KorolevKRingTopologySEQ task(input);

  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());

  auto output = task.GetOutput();
  EXPECT_EQ(output, input.data);
}

// Тест 8: Передача от среднего процесса к среднему
TEST_F(KorolevKRingTopologyFuncTest, SendBetweenMiddleProcesses) {
  int size = GetWorldSize();
  if (size < 4) {
    GTEST_SKIP() << "Need at least 4 processes";
  }

  RingMessage input;
  input.source = 1;
  input.dest = size - 2;
  input.data = {11, 22, 33, 44, 55};

  KorolevKRingTopologyMPI task(input);

  ASSERT_TRUE(task.Validation());
  ASSERT_TRUE(task.PreProcessing());
  ASSERT_TRUE(task.Run());
  ASSERT_TRUE(task.PostProcessing());

  auto output = task.GetOutput();
  EXPECT_EQ(output, input.data);
}

}  // namespace korolev_k_ring_topology
