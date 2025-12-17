# Поиск максимального элемента вектора

- Студент: Баданов Александр, группа 3823Б1ПР2
- Технология: SEQ, MPI
- Вариант: 3

## 1. Введение
Задача поиска максимального элемента в вектере является фундаментальной операцией в вычислительной математике и анализе данных. Параллельная реализация позволяет значительно ускорить обработку больших массивов данных за счет распределения вычислений между несколькими процессами.

## 2. Постановка задачи
Найти максимальный элемент в целочисленном вектере произвольного размера.

Входные данные: вектор целых чисел vector<int>
Выходные данные: целое число - максимальный элемент или INT_MIN для пустого вектора
Ограничения: вектор может быть пустым, содержать отрицательные числа, повторяющиеся значения

## 3. Базовый алгоритм (Последовательный)
```cpp
if (GetInput().empty()) {
    GetOutput() = INT_MIN;
    return true;
}

int max_elem = GetInput()[0];
for (size_t i = 1; i < GetInput().size(); i++) {
    max_elem = std::max(GetInput()[i], max_elem);
}
GetOutput() = max_elem;
```
Алгоритм последовательно перебирает все элементы вектора, обновляя значение максимального элемента при нахождении большего значения.

## 4. Схема распараллеливания
### Распределение данных
Исходный вектор рассылается на все процессы с использованием MPI_Scatterv. Каждый процесс получает свою часть данных для обработки.
Алгоритм распределения:

```cpp
int base_size = total_elem / world_size;
int remainder = total_elem % world_size;

std::vector<int> local_sizes(world_size);
std::vector<int> displacements(world_size);

MPI_Scatterv(root_data, local_sizes.data(), displacements.data(), MPI_INT,
             local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
```

### Коммуникационная схема
1. Процесс 0 (root):
- Определяет общий размер данных
- Рассылает размер всем процессам (MPI_Bcast)
- Рассылает данные всем процессам (MPI_Scatterv)
- Обрабатывает свою часть данных

2. Процессы 1..N-1:
- Получают размер данных (MPI_Bcast)
- Получают свою часть данных (MPI_Scatterv)
- Находят локальный максимум

3. Глобальная редукция:
- Все процессы участвуют в операции MPI_Allreduce с MPI_MAX
- Находится глобальный максимум

## 5. Детали реализации
### Структура кода
- `ops_mpi.cpp` - MPI реализация
- `ops_seq.cpp` - SEQ реализация
- `common.hpp` - общие типы данных
- Тесты в папках `tests/functional/` и `tests/performance/`

### Особенности реализации
- Валидация выполняется только на root процессе
- Корректная обработка пустого вектора
- Балансировка нагрузки между процессами

## 6. Экспериментальная установка
### Оборудование и ПО
- **Процессор:** Apple M1
- **ОС:** macOS 15.3.1
- **Компилятор:** clang version 21.1.5
- **Тип сборки:** release
- **MPI:** Open MPI v5.0.8

### Данные для тестирования
Функциональные тесты:
- 12 тестовых случаев
- Включают пустые векторы
- Отрицательные числа
- Граничные случаи

Производительные тесты:
- Размер данных: 1,000,000 элементов
- Диапазон значений: [-77777, 77777]

## 7. Результаты и обсуждение

### 7.1 Проверка корректности
Корректность проверялась с помощью 12 функциональных тестов, включающих:

- Обычные векторы
- Отрицательные числа
- Граничные случаи ( один элемент)
- Большие векторы (1000 элементов)

### 7.2 Производительность

| Процессы | Время, с | Ускорение | Эффективность |
|----------|-----------|-----------|---------------|
| 1 (SEQ)  |    0,21   | 1.00      | N/A           |
| 2        |    0,33   | 0,64      | 32%           |
| 4        |    0,47   | 0,45      | 23%            |
| 8        |    0,75   | 0,28      | 14%            |


## 8. Выводы
В ходе работы была успешно решена задача поиска максимального элемента вектора использованием последовательного алгоритма и технологии MPI для параллельных вычислений

- Эффективное распределение данных
- Корректная обработка граничных случаев

### Ограничения
- Накладные расходы MPI для небольших векторов
- Снижение эффективности при большом количестве процессов


## 9. Источники
1. Курс лекций по параллельному программированию Сысоева Александра Владимировича. 
2. Документация по курсу: https://learning-process.github.io/parallel_programming_course/ru

## Приложение

```cpp
bool BadanovAMaxVecElemMPI::RunImpl() {
  int rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int total_elem = 0;
  if (rank == 0) {
    total_elem = static_cast<int>(GetInput().size());
  }

  MPI_Bcast(&total_elem, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (total_elem == 0) {
    GetOutput() = INT_MIN;
    return true;
  }

  int base_size = total_elem / world_size;
  int remainder = total_elem % world_size;

  std::vector<int> local_sizes(world_size);
  std::vector<int> displacements(world_size);

  if (rank == 0) {
    int offset = 0;
    for (int i = 0; i < world_size; ++i) {
      local_sizes[i] = base_size + (i < remainder ? 1 : 0);
      displacements[i] = offset;
      offset += local_sizes[i];
    }
  }

  int local_size = base_size + (rank < remainder ? 1 : 0);
  std::vector<int> local_data(local_size);

  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, local_sizes.data(), displacements.data(), MPI_INT,
               local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);

  int max_elem_local = INT_MIN;
  for (int i = 0; i < local_size; ++i) {
    max_elem_local = std::max(local_data[i], max_elem_local);
  }

  int max_elem_global = INT_MIN;
  MPI_Allreduce(&max_elem_local, &max_elem_global, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  GetOutput() = max_elem_global;
  return true;
}
```