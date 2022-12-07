// clang-format off
#define TESTING false

#include "third_party/aoc/2022/task1.c"
#include "third_party/aoc/2022/task2.c"
#include "third_party/aoc/2022/task3.c"
#include "third_party/aoc/2022/task4.c"
#include "third_party/aoc/2022/task5.c"
#include "third_party/aoc/2022/task6.c"

int main()
{
    runTask("2022 Day 1: Calorie Counting", task1);
    runTask("2022 Day 2: Rock Paper Scissors", task2);
    runTask("2022 Day 3: Rucksack Reorganization", task3);
    runTask("2022 Day 4: Camp Cleanup", task4);
    runTask("2022 Day 5: Supply Stacks", task5);
    runTask("2022 Day 6: Tuning Trouble", task6);
    
    return 0;
}
