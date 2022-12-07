// clang-format off
#define TESTING false

#include "third_party/aoc/2022/task1.c"
#include "third_party/aoc/2022/task2.c"
#include "third_party/aoc/2022/task3.c"
#include "third_party/aoc/2022/task4.c"
#include "third_party/aoc/2022/task5.c"
#include "third_party/aoc/2022/task6.c"
#include "third_party/aoc/2022/task7.c"

// TODO Support external memory testing, I tried valgrind memcheck but it gives 0 regardless. cosmo issue?
int main()
{
    runTask("2022 Day 1: Calorie Counting", task1);
    runTask("2022 Day 2: Rock Paper Scissors", task2);
    runTask("2022 Day 3: Rucksack Reorganization", task3);
    runTask("2022 Day 4: Camp Cleanup", task4);
    runTask("2022 Day 5: Supply Stacks", task5);
    runTask("2022 Day 6: Tuning Trouble", task6);
    runTask("2022 Day 7", task7);

    return 0;
}
