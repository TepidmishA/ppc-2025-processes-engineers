
# Нахождение числа нарушений упорядоченности соседних элементов вектора

-  Student: Олесницкий Владимир Тарасович, group 3823Б1ПР2
-  Technology: SEQ | MPI
-  Variant: 6

## 1. Introduction

 - Brief motivation: познакомиться с Open MPI, освоить способы разработки параллельных программ.
 - Problem context: разработка параллельных программ представляет собой большую сложность нежели создание последовательных, однако параллельная реализация в некоторых случаях даёт ощутимый выигрыш по производительности, что делает этот инструмент необходимым для изучения. 
 - Expected outcome: я получу базовые навыки в разработке параллельных программ с помощью средств Open MPI.
## 2. Problem Statement
 - Formal task definition: необходимо написать последовательную и параллельную, использующую средства Open MPI, программы, которые позволят найти число нарушений упорядоченности соседних элементов вектора. Сравнить скорости работы    полученных реализаций, а так же проверить их валидность посредством Func и Perf тестов.
 - input/output format: на вход программе дается вектор, состоящий из чисел типа double. На выход подаётся число int, равное количеству пар соседних элементов, в котором левое число больше правого.
## 3. Baseline Algorithm (Sequential)
Если на вход подаётся вектор, чей размер не превосходит 2, программа сразу даёт ответ 0.
В ином случае, в цикле проходимся от первого элемента до предпоследнего, сравнивая текущий элемент с его правым соседом. Если результатом сравнения служит знак >, то увеличиваем счётчик неупорядоченных пар на 1. (при сравнение чисел типа double необходимо сравнивать их разность с достаточно малым числом - epsilon)

## 4. Parallelization Scheme
 -  Data distribution: 
```cpp
total_size, world_size, world_rank;
int base_chunk = total_size / world_size;
int remainder = total_size % world_size;
int my_start = world_rank * base_chunk + min(world_rank, remainder);
int my_end = my_start + base_chunk + (world_rank < remainder ? 1 : 0);
```
 - Rank roles:
	 - Process 0:
		 - Обработка маленьких массивов
		 - Рассылка данных через MPI_Scatterv и MPI_Bcast
	 -  All processes:
		 - Локальный подсчёт нарушений в своём блоке
		 - Проверка граничных элементов (кроме process 0)
		 - Участие в MPI_Allreduce
![Блок-схема](sxem.png)

## 5. Implementation Details
-  Code structure (files, key classes/functions): функции ValidationImpl, PreProcessingImpl, PostProcessingImpl по сути не используются - всегда возвращается true. Вся логика содержится в функции:
 ```cpp
bool OlesnitskiyVFindViolMPI::RunImpl() {
  if (GetInput().size() < 2) {
    GetOutput() = 0;
    return true;
  }
  
  const auto &input_data = GetInput();
  int world_size = 0;
  int world_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  
  int total_size = static_cast<int>(GetInput().size());
  
  if (total_size <= world_size) {
    return RunSequentialCase();
  }
  int base_chunk = total_size / world_size;
  int remainder = total_size % world_size;
  std::vector<int> send_counts(world_size);
  std::vector<int> displacements(world_size);
  int displacement = 0;
  for (int i = 0; i < world_size; i++) {
    send_counts[i] = base_chunk + (i < remainder ? 1 : 0);
    displacements[i] = displacement;
    displacement += send_counts[i];
  }
  int my_chunk_size = send_counts[world_rank];
  std::vector<double> local_data(my_chunk_size);
  MPI_Scatterv(input_data.data(), send_counts.data(), displacements.data(), MPI_DOUBLE, local_data.data(), my_chunk_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  std::vector<double> prev_elements(world_size, 0.0);
  if (world_rank == 0) {
    for (int i = 1; i < world_size; i++) {
      int prev_block_end = displacements[i] - 1;
      prev_elements[i] = input_data[prev_block_end];
    }
  }
  double my_prev_element = 0.0;
  MPI_Scatter(prev_elements.data(), 1, MPI_DOUBLE, &my_prev_element, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  int local_viol = 0;
  if (world_rank > 0) {
    local_viol += CountViolation(my_prev_element, local_data[0]);
  }
  for (int i = 0; i < my_chunk_size - 1; i++) {
    local_viol += CountViolation(local_data[i], local_data[i + 1]);
  }
  int total_viol = 0;
  MPI_Allreduce(&local_viol, &total_viol, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  
  GetOutput() = total_viol;
  return true;
}

bool OlesnitskiyVFindViolMPI::RunSequentialCase() {
  const auto &input_data = GetInput();
  int world_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  
  int viol = 0;
  if (world_rank == 0) {
    for (int i = 0; i < static_cast<int>(input_data.size()) - 1; i++) {
      viol += CountViolation(input_data[i], input_data[i + 1]);
    }
  }
  MPI_Bcast(&viol, 1, MPI_INT, 0, MPI_COMM_WORLD);
  GetOutput() = viol;
  return true;
}

int OlesnitskiyVFindViolMPI::CountViolation(double current, double next) const {
  const double epsilon = 1e-10;
  return (current - next > epsilon) ? 1 : 0;
}
```
-  Important assumptions and corner cases: если на вход подаётся вектор длины меньше 2 - это не считается ошибкой, при этом сразу выдаётся ответ 0.
-  Memory usage considerations: Только процесс с рангом 0 имеет доступ к GetInput(). Он рассчитывает блоки какого размера будут отправлены каждому из процессов. А так же каждому процессу, кроме процесса с рангом 0, отправляет последний элемент предыдущего блока.
## 6. Experimental Setup
-  Hardware/OS: CPU model, cores/threads, RAM, OS version:
	- Модель ЦП: AMD Ryzen 5 5600H with Radeon Graphics
	- Архитектура: x86_64
	- Ядра/потоки: 6 ядер, 12 потоков
	- ОЗУ: 16 ГБ (14 GiB доступно, 15695392 kB)
	- Версия ОС: Ubuntu 22.04.5 LTS (jammy)
	- Ядро: Linux 6.8.0-87-generic
-  Toolchain: 
	- GCC 13.1.0 (Ubuntu 13.1.0-8ubuntu1~22.04)
	- Clang 17.0.6 (AMD AOCC 5.0.0)
	- CMake 4.1.2
	- GNU Make 4.3

## 7. Results and Discussion

### 7.1 Correctness
Корректность работы была проверена с помощью комплексного модульного тестирования с использованием фреймворка Google Test. Набор тестов включал 20 тестовых случаев, охватывающих различные сценарии:
```
const std::array<TestType, 10> kTestParam = {
    std::make_tuple(std::vector<double>{}, 0, "empty"),
    std::make_tuple(std::vector<double>{1.0}, 0, "single"),
    std::make_tuple(std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0}, 0, "sorted_asc"),
    std::make_tuple(std::vector<double>{5.0, 4.0, 3.0, 2.0, 1.0}, 4, "sorted_desc"),
    std::make_tuple(std::vector<double>{1.0, 3.0, 2.0, 5.0, 4.0}, 2, "mixed"),
    std::make_tuple(std::vector<double>{1.0, 2.0, 2.0, 3.0, 3.0}, 0, "duplicates"),
    std::make_tuple(std::vector<double>{1.0, 1.0 + 1e-11, 1.0 + 1e-9}, 0, "precision_low"),
    std::make_tuple(std::vector<double>{1.0, 1.0 - 1e-8, 1.0 - 2e-8}, 2, "precision_high"),
    std::make_tuple(std::vector<double>{1.0, 2.0}, 1, "two_numbers"),
    std::make_tuple(std::vector<double>{3.0, 1.0, 2.0}, 1, "three_numbers")};
```
Все тесты прошли успешно (20/20) со временем выполнения 0 мс на тест, что подтверждает правильность подсчета нарушений для обеих реализаций: последовательной (SEQ) и MPI.


### 7.2 Performance
Present time, speedup and efficiency. Example table:

| Mode        | Count | Time, s | Speedup | Efficiency |
|-------------|-------|---------|---------|------------|
| sequential         | 1     | 0.06406 | 1.00    | N/A        |
| mpi         | 2     | 0.26673   | 0.24    | 12.0%        |
| mpi         | 3     | 0.22171   |   0.29  |   9.7%    |
| mpi         | 4     | 0.20419   |   0.31  |   7.8%    |

## 8. Conclusions
Негативное ускорение (slowdown): MPI-реализация демонстрирует замедление в 3-4 раза по сравнению с последовательной версией. При использовании 4 процессов время выполнения увеличивается с 0.06406 с до 0.20419 с (speedup = 0.31).

Эффективность параллелизма: Эффективность использования вычислительных ресурсов крайне низкая — уменьшается с 12.0% для 2 процессов до 7.8% для 4 процессов. Это свидетельствует о значительных накладных расходах параллельной реализации.

Причины низкой производительности:

Доминирование коммуникационных операций: Время передачи данных между процессами превышает выигрыш от параллельных вычислений

Низкая вычислительная плотность: Алгоритм содержит мало математических операций по сравнению с операциями ввода-вывода и коммуникации

Последовательная часть доминирует: Основная работа выполняется в процессе-мастере, который распределяет данные

Масштабируемость: С увеличением количества процессов наблюдается незначительное улучшение времени выполнения, но эффективность при этом снижается, что является признаком плохой масштабируемости для данной задачи.

Итог: Для данной конкретной задачи параллельная реализация на MPI нецелесообразна, так как приводит к значительному замедлению из-за доминирования накладных расходов коммуникации над вычислительной работой.

## 9. References
1. [Учебные материалы](https://disk.yandex.ru/d/NvHFyhOJCQU65w)