# Подсчет числа слов в строке
- Student: Борунов Владислав Алексеевич, group 3823Б1ПР3
- Technology: SEQ | MPI
- Variant: 24

## 1. Introduction
Работа посвящена реализации и анализу производительности алгоритма подсчета слов (Word Count) в длинной строке. Цель состоит в разработке параллельной версии на основе технологии Message Passing Interface (MPI) и сравнении ее эффективности с последовательной реализацией.

## 2. Problem Statement

Определение задачи:
Для заданной входной строки S требуется вычислить общее количество слов W.

Формат I/O:

Вход (I): строка символов (std::string).

Выход (O): Общее количество слов (uint64_t)

## 3. Последовательная реализация (SEQ)

Инициализация счетчика count = 0 и флага in_word = false.

Проход по каждому символу строки:
```cpp
for (char c : str) {
  if (std::isspace(c)) {
    in_word = false;
  } else {
    if (!in_word) {
      count++;        // Нашли начало нового слова
      in_word = true;
    }
  }
}
```
Возврат значения count.

## 4. Parallelization Scheme

Распараллеливание использует подход геометрической декомпозиции данных, при котором входная строка разбивается на непрерывные блоки, обрабатываемые разными процессами.

Распределение: Строка длины L делится между P процессами.

Базовый размер блока: base_count = L / P.

Остаток: remainder = L % P.

Первые remainder процессов получают base_count + 1 символов, остальные — base_count.

Обработка границ (Boundary Handling):

Каждый процесс i (кроме нулевого) запрашивает последний символ блока предыдущего процесса (i-1).

Если последний символ процесса i-1 был пробелом, а первый символ процесса i - не пробел, то процесс i засчитывает начало нового слова.

Топология:

Линейная для обмена граничными значениями, централизованная для сбора результатов.

MPI-операции коммуникации

MPI_Scatterv - распределение частей строки (массива char) неравного размера между процессами.

MPI_Sendrecv - обмен граничными символами (отправка своего последнего символа соседу справа, прием последнего символа от соседа слева).

MPI_Reduce - суммирование локальных счетчиков слов в глобальный результат (операция MPI_SUM).

MPI_Bcast - рассылка размера строки и финального результата всем процессам.

## 5. Implementation Details

### 5.1 Структура кода

Файлы:

- `common/include/common.hpp` - общие типы данных.
- `seq/include/ops_seq.hpp`, `seq/src/ops_seq.cpp` - последовательная реализация
- `mpi/include/ops_mpi.hpp`, `mpi/src/ops_mpi.cpp` - параллельная реализация
- `tests/functional/main.cpp` - функциональные тесты
- `tests/performance/main.cpp` - тесты производительности

Ключевые методы:

CalculateDistribution() - расчет send_counts и displs для MPI_Scatterv.
CountWordsLocal() - локальный подсчет слов с учетом символа от левого соседа.

### Важные решения при реализации

Управление памятью:

- Полная строка хранится только на Root-процессе (Rank 0).

- Остальные процессы выделяют память только под свой локальный фрагмент (local_data), что обеспечивает масштабируемость по памяти.

Граничные случаи:

- Пустая строка: Возвращается 0.

- Малое количество данных: Если длина строки меньше количества процессов (text_len < world_size), вычисление производится только на Rank 0, остальные процессы простаивают. Это позволяет избежать накладных расходов и ошибок деления.

## 6. Experimental Setup

### Аппаратное обеспечение

- CPU: i7-12650H  
- Ядра: 10
- Потоки: 16
- ОЗУ: 16 ГБ   
- ОС: windows 11 

### Программное обеспечение

- Компилятор: MSVC 14.44
- Сборка: Release 

### Генерация тестовых данных

Генерируемая строка: 25 000 000 повторений слова "word ".
Ожидаемый ответ: 25 000 000.

## 7. Results and Discussion

### 7.1 Correctness

- Все функциональные тесты пройдены успешно.
- Результат параллельного алгоритма на тестовых данных (25 млн слов) совпал с ожидаемым значением и результатом последовательной версии.
- Пограничные случаи покрыты функциональнымитестами.

### 7.2 Performance

Замеры времени выполнения (в секундах) для задачи размером 25 000 000 слов:

| Режим | Процессов | Время, сек | Ускорение | Эффективность |
|-------|-----------|------------|-----------|---------------|
| seq   | 1         | 1.9921     | 1.00      | N/A           |
| mpi   | 4         | 0.9315     | 2.14      | 53.5%         |

## 8. Conclusions

## Результаты:

- Реализована надежная схема обработки границ между процессами с помощью MPI_Sendrecv. Слова, разорванные между процессами, подсчитываются корректно.

- Получено ускорение в 2.14 раза на 4 процессах. Это подтверждает работоспособность параллельного подхода для данной задачи.

- Балансировка нагрузки с помощью использования MPI_Scatterv 

## Ограничения и проблемы

Низкая вычислительная сложность

Операция проверки символа (std::isspace) крайне дешевая. Время вычислений сопоставимо со временем передачи данных и синхронизации.

Эффективность 53.5% на 4 процессах указывает на высокие накладные расходы коммуникации (особенно Scatterv и Reduce) относительно полезной работы.

Масштабируемость: Для данной задачи дальнейшее увеличение числа процессов на малых объемах данных может привести к падению производительности, так как накладные расходы на пересылку данных перевесят выигрыш от параллельной обработки.

## 9. References

- Microsoft. Microsoft MPI Documentation. https://learn.microsoft.com/en-us/message-passing-interface/microsoft-mpi
- Сысоев А. В. Лекции по параллельному программированию. — Н. Новгород: ННГУ, 2025.

## Appendix 
```cpp
bool BorunovVCntWordsMPI::RunImpl() {
  int rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int text_len = 0;
  if (rank == 0) {
    text_len = static_cast<int>(GetInput().size());
  }
  MPI_Bcast(&text_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

  uint64_t global_result = 0;
  if (text_len < world_size) {
    if (rank == 0) {
      global_result = CountWordsLocal(GetInput().data(), text_len, ' ');
    }
    MPI_Bcast(&global_result, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    GetOutput() = global_result;
    return true;
  }

  std::vector<int> send_counts;
  std::vector<int> displs;
  CalculateDistribution(text_len, world_size, send_counts, displs);


  int local_count = send_counts[rank];
  std::vector<char> local_data(local_count);
  const char *send_buf = (rank == 0) ? GetInput().data() : nullptr;
  MPI_Scatterv(send_buf, send_counts.data(), displs.data(), MPI_CHAR, local_data.data(), local_count, MPI_CHAR, 0,
               MPI_COMM_WORLD);

  int left_neighbor = (rank == 0) ? MPI_PROC_NULL : rank - 1;
  int right_neighbor = (rank == world_size - 1) ? MPI_PROC_NULL : rank + 1;

  char char_to_send = ' ';
  if (local_count > 0) {
    char_to_send = local_data.back();
  }
  char prev_char = ' ';
  MPI_Sendrecv(&char_to_send, 1, MPI_CHAR, right_neighbor, 0, &prev_char, 1, MPI_CHAR, left_neighbor, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

  uint64_t local_result = 0;
  if (local_count > 0) {
    local_result = CountWordsLocal(local_data.data(), local_count, prev_char);
  }

  MPI_Reduce(&local_result, &global_result, 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Bcast(&global_result, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);

  GetOutput() = global_result;
  return true;
}
```
