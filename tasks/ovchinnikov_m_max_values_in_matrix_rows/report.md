  

# Нахождение максимальных значений по столбцам матрицы

  

- Student: Овчинников Матвей Евгеньевич, group 3823Б1ПР2

- Technology: SEQ | MPI

- Variant: 16

  

## 1. Introduction

  

- Brief motivation: узнать про технологию Open MPI, написать собственную программу для изучения работы параллельных процессов.

- Problem context: создать программу, которая использует параллельные процессы, сложнее, чем последовательную. Однако, это окупается высокой производительностью, однако, в ходе работы я выяснил, что накладные расходы параллельной версии могут значительно замедлить программу.

- Expected outcome: получить навыки работы с параллельными процессами, изучить внутренности MPI и как устроена архитектура параллелизма.

## 2. Problem Statement

- Formal task definition: нужно написать последовательную и параллельную, использующую средства Open MPI, программы, которые позволят найти максимальное значение в каждом столбце введенной матрицы. Сравнить скорости работы полученных реализаций через perfomance тесты, а так же проверить их валидность посредством functional тестов.

- input/output format: на вход программе подаются размеры матрицы (два size_t числа), и сама матрица, которая представлена в виде одного вектора, содержащего int числа (то есть матрица хранится линейно). На выход подаётся вектор, с числами int, которые являются максимальными значениями столбцов матрицы (j-ый элемент вектора равен максимальному значению j-го столбца в данной матрице).

## 3. Baseline Algorithm (Sequential)

Если на вход подается пустая матрица, то программа вернет пустой вектор. В случае, если матрица имеет хотя бы одну ячейку, то мы проходимся внешним циклом по столбцам матрицы, а внутренними циклами по каждому столбцу отдельно и ищем в нём максимальный по значению элемент, который хранится в векторе.

## 4. Parallelization Scheme

- Data distribution:

Данные делятся нулевым процессом между остальными следующим образом:
1. сначала высчитывается базовый размер кусочка, который достанется каждому процессу. (Мы делим кол-во строк (да-да именно строк, т.к матрица то хранится одним сплошным вектором) на кол-во процессов)
2. после этого есть шанс, что кол-во процессов не кратно кол-ву строк, следовательно нужно достать остаток строк, которые мы потеряли, для этого берем остаток от деления строк на процессы.
3. Чтобы i-ый процесс знал, где он должен работать, ему нужно знать где начальная граница его блока данных, для этого переступим через все блоки процессов, которые предшествуют процессу i и прибавим минимум из номера процесса и остатка строк.
4. Для понимания конца своего блока достаточно прибавить к началу размер блока и в зависимости от того, меньше ли номер процесса, чем остаток, мы прибавляем единицу или ноль соответственно.

Этот алгоритм широко известен в HPC-литературе под несколькими именами, но его суть в том, чтобы распределить данные между процессами как можно равномернее, то есть по итогу его работы любые два блока данных не отличаются размерами больше, чем на единицу.

```cpp

size_t chunk_base = lines / process_amount;

size_t chunk_extra = lines % process_amount;

size_t chunk_start = (rank * chunk_base) +  std::min(rank, chunk_extra);

size_t chunk_end_bit=0;
if(rank<chunk_extra) chunk_end_bit=1;
size_t chunk_end = chunk_start + chunk_base + chunk_end_bit;

```

- Rank roles:

- Process 0:

Нулевой процесс использует алгоритм распределения данных по блокам для других процессов и кладет эти данные в два вектора:
в первом векторе elem_count в i-ой ячейке хранится кол-во элементов, которое должен обрабатывать i-ый процесс; во втором векторе elem_offset в i-ой ячейке хранится смещение, чтобы i-ый процесс понимал откуда ему нужно стартовать.

```cpp
  std::vector<int> elem_count(proc_amount);
  std::vector<int> elem_offset(proc_amount);
  if (rank == 0) {
    const size_t chunk_base = lines / proc_amount;
    const size_t chunk_extra = lines % proc_amount;
    for (size_t process = 0; process < proc_amount; process++) {
      const size_t line_begin = (process * chunk_base) + std::min(process, chunk_extra);
      size_t line_end = line_begin + chunk_base;
      if(process < chunk_extra){
        line_end++;
      }

      size_t elems = (line_end - line_begin) * cols;
      size_t offset = line_begin * cols;
      elem_count[process] = static_cast<int>(elems);
      elem_offset[process] = static_cast<int>(offset);
    }
  }
```
p.s - на нулевом процессе я заполняю изначально пустой буфер для матрицы оригинальными данными, который потом отправится в scatterv.
```cpp
 if(rank==0){
    matrix_buffer=matrix.data();
  }
```

- Рассылка данных через MPI_Bcast

```cpp
  MPI_Bcast(elem_count.data(), static_cast<int>(proc_amount), MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(elem_offset.data(), static_cast<int>(proc_amount), MPI_INT, 0, MPI_COMM_WORLD);
```

- All processes:

- Локальный поиск максимума в своём блоке
```cpp
  std::vector<int> local_max(cols, std::numeric_limits<int>::min());

  size_t local_lines = local_data.size() / cols;

  for (size_t i = 0; i < local_lines; i++) {
    const size_t local_base = i * cols;
    for (size_t j = 0; j < cols; j++) {
      local_max[j] = std::max(local_max[j], local_data[local_base + j]);
    }
  } 
```
- Участие в MPI_Allreduce

```cpp
  MPI_Allreduce(local_max.data(), global_max.data(),
                static_cast<int>(cols), MPI_INT, MPI_MAX, MPI_COMM_WORLD);
```


  

## 5. Implementation Details

- Code structure (files, key classes/functions): функции ValidationImpl, PreProcessingImpl, PostProcessingImpl по сути не используются - всегда возвращается true. Вся логика содержится в функции:

```cpp

bool OvchinnikovMMaxValuesInMatrixRowsMPI::RunImpl() {
  int tmp_rank = 0;
  int tmp_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &tmp_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &tmp_size);
  size_t rank = static_cast<size_t>(tmp_rank);
  size_t size = static_cast<size_t>(tmp_size);

  const size_t lines = std::get<0>(GetInput());
  const size_t cols = std::get<1>(GetInput());
  if (lines == 0 || cols == 0) {
    return true;
  }
  const auto& matrix = std::get<2>(GetInput());

  std::vector<int> elem_count(size);
  std::vector<int> elem_offset(size);
  if (rank == 0) {
    const size_t base = lines / size;
    const size_t extra = lines % size;
    for (size_t process = 0; process < size; process++) {
      const size_t line_begin = (process * base) + std::min(process, extra);
      size_t line_end = line_begin + base;
      if(process < extra){
        line_end++;
      }

      size_t elems = (line_end - line_begin) * cols;
      size_t offset = line_begin * cols;
      elem_count[process] = static_cast<int>(elems);
      elem_offset[process] = static_cast<int>(offset);
    }
  }

  MPI_Bcast(elem_count.data(), static_cast<int>(size), MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(elem_offset.data(), static_cast<int>(size), MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> local_data(elem_count[rank]);

  const int* matrix_buffer = NULL;
  if(rank==0){
    matrix_buffer=matrix.data();
  }
  MPI_Scatterv(
      matrix_buffer, // only for rank 0
      elem_count.data(),
      elem_offset.data(),
      MPI_INT,
      local_data.data(),  
      elem_count[rank],
      MPI_INT,
      0,
      MPI_COMM_WORLD);

  std::vector<int> local_max(cols, std::numeric_limits<int>::min());

  size_t local_lines = local_data.size() / cols;

  for (size_t i = 0; i < local_lines; i++) {
    const size_t base = i * cols;
    for (size_t j = 0; j < cols; j++) {
      local_max[j] = std::max(local_max[j], local_data[base + j]);
    }
  } 
  std::vector<int> global_max(cols);
  MPI_Allreduce(local_max.data(), global_max.data(),
                static_cast<int>(cols), MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  GetOutput() = global_max;
  return true;
}

```

- Important assumptions and corner cases: если на вход подаётся пустая матрица, то возвращается true

- Memory usage considerations: исходные данные доступны исключительно нулевому процессу, который потом рассылает всем остальным данные о положении их блоков и сами эти блоки данных.

  

## 6. Experimental Setup

- Hardware/OS: CPU model, cores/threads, RAM, OS version:

- Модель ЦП: 13th Gen Intel(R) Core(TM) i7-13700H

- Архитектура: x86_64

- Ядра/потоки: 6 ядер, 12 потоков

- ОЗУ: 16 ГБ

- Версия ОС: WSL: 2.6.1.0

- Ядро: Linux 6.8.0-87-generic

- Toolchain:

- GCC 13.1.0 (Ubuntu 13.1.0-8ubuntu1~22.04)

- Clang 18.1.3

- CMake 3.28.3

- GNU Make 4.3

  

## 7. Results and Discussion

### 7.1 Correctness

Корректность работы была проверена с помощью комплексного модульного тестирования с использованием фреймворка Google Test. Набор тестов включает в себя 6 тестовых случаев, охватывающих различные сценарии:

```

const std::array<TestType, 6> kTestParam = {
    std::make_tuple(0, 0, std::vector<int>{}, "empty_matrix"),

    std::make_tuple(3, 3, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}, "matrix_3x3"),

    std::make_tuple(2, 2, std::vector<int>{-1, -5, 4, 0}, "negatives"),

    std::make_tuple(1, 1, std::vector<int>{10}, "single_element"),

    std::make_tuple(2, 3, std::vector<int>{1, 10, 3, 7, 0, 100}, "random"),

    std::make_tuple(5, 9, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 7, 6, 3, 2, 9, 4, 2, 5, 8, 1, 2, 3, 4, 5,
                                           6, 7, 8, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9, 7, 6, 3, 2, 9, 4, 2, 5, 8},
                    "large_matrix"),
};

```

Все тесты прошли успешно (6/6) со временем выполнения 0-2 мс на тест, что подтверждает правильность подсчета максимума столбцов матрицы для обеих реализаций: SEQ и MPI.

  
  

### 7.2 Performance

Present time, speedup and efficiency. Example table:

| Mode | Count | Time, s         | Speedup | Efficiency |
| ---- | ----- | --------------- | ------- | ---------- |
| seq  | 1     | 0.02 ± 0.005    | 1.00    | 1          |
| seq  | 1     | 0.02 ± 0.005    | 1.00    | 1          |
| mpi  | 2     | 0.0765 ± 0.0025 | 0.261   | 0.131      |
| mpi  | 2     | 0.078 ± 0.002   | 0.256   | 0.128      |
| mpi  | 3     | 0.063 ± 0.003   | 0.317   | 0.106      |
| mpi  | 3     | 0.065 ± 0.005   | 0.308   | 0.103      |
| mpi  | 4     | 0.057 ± 0.0025  | 0.351   | 0.0877     |
| mpi  | 4     | 0.057 ± 0.0025  | 0.351   | 0.0877     |
| mpi  | 6     | 0.044 ± 0.004   | 0.455   | 0.0758     |
| mpi  | 6     | 0.05 ± 0.01     | 0.400   | 0.0667     |
| mpi  | 8     | 0.048 ± 0.002   | 0.417   | 0.0521     |
| mpi  | 8     | 0.06 ± 0.015    | 0.333   | 0.0417     |

Для чистоты эксперимента я запускал по 5 раз перфоманс тесты для каждого количества процессов, я брал минимальное и максимальное получившееся значение, находил среднее и подписал погрешность тестирования. (Заметил, что чем больше процессов, тем больше погрешность во времени).

К большому разочарованию, можно заметить, что параллелизм не только не ускорил решение задачи, но ещё и значительно замедлил.
Скорость достигает пика к 6 процессам, но эффективность их использования при этом неумолимо мала. Последовательная версия по всем параметрам выиграла эту гонку. Далее в выводе я объясню почему так получилось. 

  
  

## 8. Conclusions

Summarize findings and limitations.

  Первоначальная архитектура, в которой все процессы имели доступ ко всей памяти, была значительно изменена. В текущей версии нулевой процесс вычисляет размеры и положение блоков данных, а затем рассылает эти блоки остальным процессам, которые, в свою очередь, занимаются поиском максимума в своём "чанке".

  Новая архитектура заметно замедлила работу MPI версии, это связано с тем, что перед выполнением основной работы все процессы должны дождаться завершения работы нулевого процесса. 
  
  Помимо этого, я столкнулся со следующей проблемой: матрица делится на блоки по строкам, но максимум ищется в столбцах. То есть, если процессу x досталось n строк, то ему необходимо найти m максимумов в каждом блоке размером n. 
  Например, матрица размером 20 строк на 40 столбцов обрабатывается 4-мя процессами. Следовательно каждому процессу достается блок по 5 строк, а значит каждый процесс ищет максимум в массиве из 5 элементов 40 раз. Это очень неэффективно, учитывая, что в конце необходимо собрать данные, соединить их и вновь определить максимум из собранных элементов.

  Безусловно, матрицу можно было транспонировать перед параллельной обработкой, но эта операция использует в два раза больше памяти, чем занимает исходная матрица, а также требует временных ресурсов. Второе решение - подавать на вход матрицу, которая уже повернута на 90 градусов, однако, я считаю это нереалистичным сценарием использования моей программы и это сильно усложняет её применение.

  Подводя итоги, мне кажется, что данная задача не может быть ускорена с помощью параллелизма и на это есть объективные причины.
  

## 9. References

1. [Учебные материалы](https://disk.yandex.ru/d/NvHFyhOJCQU65w)