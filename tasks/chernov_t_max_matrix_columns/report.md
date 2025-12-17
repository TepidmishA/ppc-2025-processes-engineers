# Нахождение максимальных значений по столбцам матрицы

- **Студент**: Чернов Тимур, группа 3823Б1ПР1
- **Технология**: SEQ | MPI  
- **Вариант**: 16

## 1. Введение

Задача поиска максимальных элементов по столбцам матрицы широко применяется в анализе данных и численных методах. При работе с большими объёмами данных последовательные алгоритмы не обеспечивают достаточной производительности. В данной работе мною были реализованы и сравнены последовательная (SEQ) и распределённая (MPI) версии алгоритма нахождения максимальных значений матрицы по столбцам.

## 2. Постановка задачи
**Описание задачи**

В заданной матрице размера `m на n` найти значение максимального элемента для каждого.

Входные типы данных: размеры матрицы m на n типа size_t и вектор элементов матрицы типа int.
```cpp
using InType = std::tuple<std::size_t, std::size_t, std::vector<int>>;
```
Выходной тип данных: вектор значений типа int.
```cpp
using OutType = std::vector<int>;
```

### Ограничения:

- Матрица представлена в виде одномерного вектора с row-major порядком хранения  
- Матрица должна быть прямоугольной (все строки одинаковой длины)  
- Матрица не может быть пустой  
- Размеры матрицы должны соответствовать количеству элементов в векторе данных  
- Алгоритм должен корректно обрабатывать матрицы различных размерностей 

## 3. Базовый последовательный алгоритм (SEQ)

Последовательный базовый алгоритм выполняет обход каждого столбца матрицы и находит максимальный элемент путем последовательного сравнения всех элементов в данном столбце.

Код алгоритма:

```cpp
bool ChernovTMaxMatrixColumnsSEQ::RunImpl() {
    if (!valid_) {
        return false;
    }

    std::vector<int> result(cols_);

    for (std::size_t col = 0; col < cols_; ++col) {
        int max_val = input_matrix_[col];
        for (std::size_t row = 1; row < rows_; ++row) {
            std::size_t index = (row * cols_) + col;
            max_val = std::max(input_matrix_[index], max_val);
        }
        result[col] = max_val;
    }

    GetOutput() = result;
    return true;
}
```

## 4. Схема распараллеливания (MPI)

Параллельная реализация использует распределение столбцов матрицы между процессами с помощью `MPI_Scatterv`. Процесс 0 отправляет данные частями другим процессам, что соответствует требованию преподавателя.

**Этапы выполнения:**

1. Рассылка метаданных: процесс 0 передаёт размеры матрицы m и n всем процессам через `MPI_Bcast`.
2. Распределение данных: процесс 0 переупорядочивает матрицу в column-major формат и отправляет столбцы частями другим процессам с помощью `MPI_Scatterv`.
3. Локальные вычисления: каждый процесс вычисляет максимумы только для полученных столбцов.
4. Глобальная редукция: с помощью `MPI_Gatherv` все процессы отправляют свои локальные максимумы процессу 0.
5. Рассылка финального результата: процесс 0 раздаёт итоговый вектор всем процессам через `MPI_Bcast`.

**Детальный алгоритм параллельной реализации**

Рассылка размеров матрицы:

```cpp
void ChernovTMaxMatrixColumnsMPI::BroadcastDimensions(int rank) {
    std::array<int, 2> dimensions{};
    if (rank == 0) {
        dimensions[0] = static_cast<int>(rows_);
        dimensions[1] = static_cast<int>(cols_);
    }
    MPI_Bcast(dimensions.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);
    total_rows_ = dimensions[0];
    total_cols_ = dimensions[1];
}
```

Распределение данных между процессами:

```cpp
std::vector<int> ChernovTMaxMatrixColumnsMPI::ScatterMatrixData(int rank, int size) {
  int base_cols = total_cols_ / size;
  int remainder = total_cols_ % size;

  int my_cols = base_cols;
  if (rank < remainder) {
    my_cols++;
  }

  int my_elements = my_cols * total_rows_;
  std::vector<int> local_data(my_elements);

  std::vector<int> send_counts(size, 0);
  std::vector<int> displacements(size, 0);
  std::vector<int> reordered_data;

  if (rank == 0) {
    reordered_data.resize(static_cast<std::size_t>(total_rows_) * static_cast<std::size_t>(total_cols_));
    for (int col = 0; col < total_cols_; ++col) {
      for (int row = 0; row < total_rows_; ++row) {
        reordered_data[(static_cast<std::size_t>(col) * static_cast<std::size_t>(total_rows_)) +
                       static_cast<std::size_t>(row)] =
            input_matrix_[(static_cast<std::size_t>(row) * static_cast<std::size_t>(total_cols_)) +
                          static_cast<std::size_t>(col)];
      }
    }

    int current_displacement = 0;
    for (int i = 0; i < size; ++i) {
      int cols_for_i = base_cols;
      if (i < remainder) {
        cols_for_i++;
      }
      send_counts[i] = cols_for_i * total_rows_;
      displacements[i] = current_displacement;
      current_displacement += send_counts[i];
    }
  }

  MPI_Bcast(send_counts.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(displacements.data(), size, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    MPI_Scatterv(reordered_data.data(), send_counts.data(), displacements.data(), MPI_INT, local_data.data(),
                 my_elements, MPI_INT, 0, MPI_COMM_WORLD);
  } else {
    std::vector<int> dummy_sendbuf;
    MPI_Scatterv(dummy_sendbuf.data(), send_counts.data(), displacements.data(), MPI_INT, local_data.data(),
                 my_elements, MPI_INT, 0, MPI_COMM_WORLD);
  }

  return local_data;
}
```

Локальные вычисления максимумов:

```cpp
std::vector<int> ChernovTMaxMatrixColumnsMPI::ComputeLocalMaxima(int rank, int size,
                                                                 const std::vector<int> &local_data) const {
  int base_cols = total_cols_ / size;
  int remainder = total_cols_ % size;

  int my_cols = base_cols;
  if (rank < remainder) {
    my_cols++;
  }

  std::vector<int> local_maxima(my_cols);

  for (int local_col = 0; local_col < my_cols; ++local_col) {
    int max_val = local_data[(static_cast<std::size_t>(local_col) * static_cast<std::size_t>(total_rows_))];

    for (int row = 1; row < total_rows_; ++row) {
      int element = local_data[(static_cast<std::size_t>(local_col) * static_cast<std::size_t>(total_rows_)) +
                               static_cast<std::size_t>(row)];
      max_val = std::max(element, max_val);
    }
    local_maxima[local_col] = max_val;
  }

  return local_maxima;
}
```

Сбор и рассылка результатов:

```cpp
void ChernovTMaxMatrixColumnsMPI::ComputeAndBroadcastResult(const std::vector<int> &local_maxima) {
  int size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int base_cols = total_cols_ / size;
  int remainder = total_cols_ % size;

  std::vector<int> recv_counts(size);
  std::vector<int> displacements(size);

  int current_displacement = 0;
  for (int i = 0; i < size; ++i) {
    recv_counts[i] = base_cols;
    if (i < remainder) {
      recv_counts[i]++;
    }
    displacements[i] = current_displacement;
    current_displacement += recv_counts[i];
  }

  std::vector<int> result(total_cols_);

  MPI_Gatherv(local_maxima.data(), static_cast<int>(local_maxima.size()), MPI_INT, result.data(), recv_counts.data(),
              displacements.data(), MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Bcast(result.data(), total_cols_, MPI_INT, 0, MPI_COMM_WORLD);
  GetOutput() = result;
}
```

## 5. Детали реализации

### Структура проекта:

- `common.hpp` — определение типов данных  
- `ops_mpi.hpp` — объявление MPI-класса  
- `ops_mpi.cpp` — реализация MPI-алгоритма  
- `ops_seq.hpp` — объявление SEQ-класса  
- `ops_seq.cpp` — реализация SEQ-алгоритма  
- `functional/main.cpp` — функциональные тесты  
- `performance/main.cpp` — тесты производительности 

## 6. Экспериментальная среда

**Аппаратное обеспечение:**

- Процессор: AMD Ryzen 5 5500U with Radeon Graphics  
- Тактовая частота: 2.10 GHz  
- Ядра/потоки: 6 ядер / 12 потоков  
- Оперативная память: 8 GB DDR4  
- ОС: Windows 11 и Ubuntu 24.04  

**Программное обеспечение:**

- Компилятор: GCC 13.3.0  
- MPI: Open MPI 4.1.6  
- Стандарт: C++20  
- Тип сборки: Release  

**Тестовые данные:**

- Функциональные тесты: матрицы 3×3 и 2×2 с известными результатами  
- Производительность: матрица 7000×7000 (49 миллионов элементов)

## 7. Результаты и обсуждение

### 7.1 Корректность

Все функциональные тесты успешно пройдены. Обе реализации выдают идентичные результаты для тестовых матриц. Тестовыми данными являлись матрицы 2 на 2 и 3 на 3 в двух файлах matrix_1.txt и matrix_2.txt содержащие размер матрицы и элементы из которых состояла матрица.

### 7.2 Производительность

Измерения выполнены на матрице `7000 на 7000` (согласно коду `perf_tests.cpp`). Время — значение из лога `task_run`. За базовое время SEQ принято значение при запуске в однопроцессном режиме: **1.5219 с**.

| Режим | Число процессов | Время, с | Ускорение | Эффективность |
|-------|------------------|----------|-----------|----------------|
| seq   | 1                | 1.5219   | 1.00      | N/A            |
| mpi   | 2                | 1.0491   | 1.45      | 72.5%          |
| mpi   | 3                | 0.9659   | 1.58      | 52.6%          |
| mpi   | 4                | 1.3133   | 1.16      | 29.0%          |

## 8. Выводы

Задача нахождения максимальных элементов по столбцам матрицы была успешно реализована в двух вариантах: **последовательном (SEQ)** и **распределённом (MPI)** с использованием библиотеки **Open MPI**. 

Для проверки корректности разработаны функциональные тесты на малых матрицах (2×2 и 3×3), а для оценки производительности использовалась матрица размером `7000×7000`. Эксперименты показали, что MPI-реализация демонстрирует **максимальное ускорение 1.58× при 3 процессах**, а наилучшая параллельная эффективность (**72.5%**) достигается при использовании **2 процессов**.

## 9. Источники

1. **Курс лекций по параллельному программированию** Сысоев А. В.

2. **Технологии параллельного программирования MPI и OpenMP** А.В. Богданов, В.В. Воеводин и др., - МГУ, 2012.

3. **Документация Open MPI:** https://www.open-mpi.org/

4. **Microsoft MPI Functions:** https://learn.microsoft.com/ru-ru/message-passing-interface/mpi-functions