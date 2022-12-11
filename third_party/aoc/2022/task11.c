// clang-format off
#include "libc/inttypes.h"
#include "libc/fmt/conv.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"
#include "third_party/aoc/utilities/dynamicarray/ary.h"

static char MonkeyIndexText[] =    "Monkey ";
static char StartingItemsText[] =  "  Starting items: ";
static char OperationText[] =      "  Operation: new = old ";
static char OperationOldText[] =   "old\n";
static char WorryTestText[] =      "  Test: divisible by ";
static char WorryTestTrueText[] =  "    If true: throw to monkey ";
static char WorryTestFalseText[] = "    If false: throw to monkey ";

enum Operation
{
    Add,
    AddOld,
    Mult,
    MultOld
};

struct MonkeyInfo
{
    uint_fast8_t monkeyIndex;
    
    struct ary(uint_fast64_t) currentWorryLevels;
    
    enum Operation operation;
    uint_fast8_t operationOperand;
    
    uint_fast8_t worryDivisibleValue;
    uint_fast8_t worryTestTrueMonkey;
    uint_fast8_t worryTestFalseMonkey;
    
    uint_fast32_t inspectionCount;
}; 

struct array_MonkeyInfo ary(struct MonkeyInfo*);

// Indicates section currently being looked for
enum ParsingSection
{
    MonkeyIndex,
    StartingItems,
    Operation,
    WorryTest,
    WorryTestTrue,
    WorryTestFalse,
    BlankLine
};

uint_fast64_t PerformOperation(struct MonkeyInfo* info, uint_fast64_t worryLevel)
{
    switch (info->operation)
    {
        case Add:
        return info->operationOperand + worryLevel;
        case AddOld:
        return worryLevel + worryLevel;
        case Mult:
        return info->operationOperand * worryLevel;
        case MultOld:
        return worryLevel * worryLevel;
    }
        
    perror("Unable to parse input file");
    exit(1);
}

void PerformRound(struct array_MonkeyInfo* monkeyInfos)
{
    for (size_t i = 0; i < monkeyInfos->len; i++)
    {
        struct MonkeyInfo* info = monkeyInfos->buf[i];
        
        while(info->currentWorryLevels.len > 0)
        {
            uint_fast64_t worryLevel = info->currentWorryLevels.buf[0];
            
            worryLevel = PerformOperation(info, worryLevel);

            worryLevel /= 3;
            
            uint_fast8_t newMonkey;
            if (worryLevel % info->worryDivisibleValue == 0)
            {
                newMonkey = info->worryTestTrueMonkey;
            }
            else
            {
                newMonkey = info->worryTestFalseMonkey;
            }
            info->inspectionCount++;
            
            ary_remove(&info->currentWorryLevels, 0);
            ary_insert(&monkeyInfos->buf[newMonkey]->currentWorryLevels, monkeyInfos->buf[newMonkey]->currentWorryLevels.len, worryLevel);
        }
    }
}

void PerformFullRound(struct array_MonkeyInfo* monkeyInfos)
{
    for (size_t i = 0; i < monkeyInfos->len; i++)
    {
        struct MonkeyInfo* info = monkeyInfos->buf[i];
        
        printf("Monkey %i:\n", info->monkeyIndex);
        while(info->currentWorryLevels.len > 0)
        {
            uint_fast64_t worryLevel = info->currentWorryLevels.buf[0];
            printf("  Monkey inspects an item with a worry level of %" PRIuFAST16 ".\n", worryLevel);
            
            worryLevel = PerformOperation(info, worryLevel);
            switch (info->operation)
            {
                case Add:
                printf("    Worry level increases by %" PRIuFAST8 " to ", info->operationOperand);
                break;
                case AddOld:
                
                break;
                case Mult:
                printf("    Worry level is multiplied by %" PRIuFAST8 " to ", info->operationOperand);
                break;
                case MultOld:
                printf("    Worry level is multiplied by itself to ");
                break;
            }
            
            printf("%" PRIuFAST16 ".\n", worryLevel);
            
            worryLevel /= 3;
            printf("    Monkey gets bored with item. Worry level is divided by 3 to %" PRIuFAST16 ".\n", worryLevel);
            
            uint_fast8_t newMonkey;
            if (worryLevel % info->worryDivisibleValue == 0)
            {
                printf("    Current worry level is divisible by %" PRIuFAST8 ".\n", info->worryDivisibleValue);
                
                newMonkey = info->worryTestTrueMonkey;
            }
            else
            {
                printf("    Current worry level is not divisible by %" PRIuFAST8 ".\n", info->worryDivisibleValue);
                
                newMonkey = info->worryTestFalseMonkey;
            }
            info->inspectionCount++;
            
            printf("    Item with worry level %" PRIuFAST16 " is thrown to monkey %" PRIuFAST8 ".\n", worryLevel, newMonkey);
            ary_remove(&info->currentWorryLevels, 0);
            ary_insert(&monkeyInfos->buf[newMonkey]->currentWorryLevels, monkeyInfos->buf[newMonkey]->currentWorryLevels.len, worryLevel);
        }
    }
}

void DisplayRoundOutcome(struct array_MonkeyInfo* monkeyInfos)
{
    for (size_t i = 0; i < monkeyInfos->len; i++)
    {
        struct MonkeyInfo* info = monkeyInfos->buf[i];

        printf("Monkey %" PRIuFAST8 ": ", info->monkeyIndex);
        
        for (size_t i = 0; i < info->currentWorryLevels.len; i++)
        {
            printf("%s %i", i != 0 ? "," : "", info->currentWorryLevels.buf[i]);
        }
        
        putchar('\n');
    }
}

//TODO Support performance testing
void task11()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Long enough for the longest input I can see which is "  Starting items", then a guess of at max 10 items per monkey with each item conisting of a delimiting char("," in general but ":" for the first one which is 4 chars each and then a new line and a terminating char
    char chunk[58];
    
    enum ParsingSection nextSection = MonkeyIndex;
    uint_fast8_t currentMonkeyIndex = 0;
    struct array_MonkeyInfo monkeyInfos;
    ary_init(&monkeyInfos, 10);
    
    uint_fast64_t monkeyBusiness = 0;
    
    while(fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        if (nextSection == MonkeyIndex && strncmp(MonkeyIndexText, chunk, sizeof(MonkeyIndexText) - 1) == 0)
        {
            char* integerStartPtr = chunk + sizeof(MonkeyIndexText) - 1;
            char* integerEndPtr;
            currentMonkeyIndex = strtoul(integerStartPtr, &integerEndPtr, 10);
            if (integerEndPtr == integerStartPtr || integerEndPtr == NULL || integerEndPtr[0] != ':' || (integerEndPtr[1] != '\n' && integerEndPtr[1] != '\0'))
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            struct MonkeyInfo* info = malloc(sizeof *info);
            info->monkeyIndex = currentMonkeyIndex;
            info->inspectionCount = 0;
            ary_init(&info->currentWorryLevels, 5);
            
            ary_push(&monkeyInfos, info);
            
            printf("Found start of monkey section: %i\n", currentMonkeyIndex);
            
            nextSection = StartingItems;
            continue;
        }
        
        if (nextSection == StartingItems && strncmp(StartingItemsText, chunk, sizeof(StartingItemsText) - 1) == 0)
        {
            char* integerStartPtr = chunk + sizeof(StartingItemsText) - 1;
            char* integerEndPtr = NULL;
            
            printf("Found starting items for monkey %i:", currentMonkeyIndex);
            
            while(integerEndPtr == NULL || *integerEndPtr != '\n')
            {
                uint_fast64_t worryLevel = strtoul(integerStartPtr, &integerEndPtr, 10);
                if (integerEndPtr == integerStartPtr || integerEndPtr == NULL)
                {
                    perror("Unable to parse input file");
                    exit(1);
                }
                
                //printf("Found worry level: %i\n", worryLevel);
                
                integerStartPtr = integerEndPtr +  2; // Skip ", "
                ary_push(&monkeyInfos.buf[currentMonkeyIndex]->currentWorryLevels, worryLevel);
            }
            
            for (size_t i = 0; i < monkeyInfos.buf[currentMonkeyIndex]->currentWorryLevels.len; i++)
            {
                printf("%s %i", i != 0 ? "," : "", monkeyInfos.buf[currentMonkeyIndex]->currentWorryLevels.buf[i]);
            }
            
            putchar('\n');
            
            nextSection = Operation;
            continue;
        }
        
        if (nextSection == Operation && strncmp(OperationText, chunk, sizeof(OperationText) - 1) == 0)
        {
            printf("Found operation for monkey %i: ", currentMonkeyIndex);
            
            enum Operation op;
            bool usesOld = false;
            uint_fast8_t operand = 0;
            
            char* startPtr = chunk + sizeof(OperationText) - 1;
            
            //printf("\n%s", startPtr + 2);
            if (strncmp(OperationOldText, startPtr + 2, sizeof(OperationOldText) - 1) == 0)
            {
                usesOld = 2;
            }
            
            switch (*startPtr)
            {
                case '+':
                op = usesOld ? AddOld : Add;
                printf("addition ");
                break;
                case '*':
                op = usesOld ? MultOld : Mult;
                printf("multiplication ");
                break;
                default:
                perror("Unable to parse input file");
                exit(1);
                break;
            }
            
            startPtr += 2;
            
            if (usesOld)
            {
                puts("with operand old");
            }
            else
            {
                char* integerEndPtr = NULL;
                operand = strtoul(startPtr, &integerEndPtr, 10);
                if (integerEndPtr == startPtr || startPtr == NULL)
                {
                    perror("Unable to parse input file");
                    exit(1);
                }
                
                printf("with operand %i\n", operand);
            }
                        
            monkeyInfos.buf[currentMonkeyIndex]->operation = op;
            monkeyInfos.buf[currentMonkeyIndex]->operationOperand = operand;
            
            nextSection = WorryTest;
            continue;
        }
        
        if (nextSection == WorryTest && strncmp(WorryTestText, chunk, sizeof(WorryTestText) - 1) == 0)
        {
            printf("Found worry test ");
            
            char* integerStartPtr = chunk + sizeof(WorryTestText) - 1;
            char* integerEndPtr;
            uint_fast8_t worryDivisibleValue = strtoul(integerStartPtr, &integerEndPtr, 10);
            if (integerEndPtr == integerStartPtr || integerEndPtr == NULL || (integerEndPtr[1] != '\n' && integerEndPtr[1] != '\0'))
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            printf("with divisible value: %i\n", worryDivisibleValue);
            
            monkeyInfos.buf[currentMonkeyIndex]->worryDivisibleValue = worryDivisibleValue;
            
            nextSection = WorryTestTrue;
            continue;
        }
        
        if (nextSection == WorryTestTrue && strncmp(WorryTestTrueText, chunk, sizeof(WorryTestTrueText) - 1) == 0)
        {
            printf("Found worry test true outcome: chunk to monkey ");
            
            char* integerStartPtr = chunk + sizeof(WorryTestTrueText) - 1;
            char* integerEndPtr;
            uint_fast8_t worryTestTrueMonkey = strtoul(integerStartPtr, &integerEndPtr, 10);
            if (integerEndPtr == integerStartPtr || integerEndPtr == NULL || (integerEndPtr[1] != '\n' && integerEndPtr[1] != '\0'))
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            printf("%i\n", worryTestTrueMonkey);
            
            monkeyInfos.buf[currentMonkeyIndex]->worryTestTrueMonkey = worryTestTrueMonkey;
            
            nextSection = WorryTestFalse;
            continue;
        }
        
        if (nextSection == WorryTestFalse && strncmp(WorryTestFalseText, chunk, sizeof(WorryTestFalseText) - 1) == 0)
        {
            printf("Found worry test false outcome: chunk to monkey ");
            
            char* integerStartPtr = chunk + sizeof(WorryTestFalseText) - 1;
            char* integerEndPtr;
            uint_fast8_t worryTestFalseMonkey = strtoul(integerStartPtr, &integerEndPtr, 10);
            if (integerEndPtr == integerStartPtr || integerEndPtr == NULL || (integerEndPtr[1] != '\n' && integerEndPtr[1] != '\0'))
            {
                perror("Unable to parse input file");
                exit(1);
            }
            
            printf("%i\n", worryTestFalseMonkey);
            
            monkeyInfos.buf[currentMonkeyIndex]->worryTestFalseMonkey = worryTestFalseMonkey;
            
            nextSection = BlankLine;
            continue;
        }
        
        if (nextSection == BlankLine)
        {
            putchar('\n');
            nextSection = MonkeyIndex;
            continue;
        }
        
        printf(chunk);
        
        perror("Unable to parse input file");
        exit(1);
    }
    
    putchar('\n');
    for (int i = 1; i <= 20; i++)
    {
        PerformRound(&monkeyInfos);
        printf("\nAfter round %i, the monkeys are holding items with these worry levels:\n", i);
        DisplayRoundOutcome(&monkeyInfos);
        
        putchar('\n');
    }
    
    uint_fast32_t highestInspectionCount = 0;
    uint_fast32_t secondHighestInspectionCount = 0;
    
    putchar('\n');
    for (size_t i = 0; i < monkeyInfos.len; i++)
    {
        struct MonkeyInfo* info = monkeyInfos.buf[i];
        
        printf("Monkey %" PRIuFAST16 " inspected items %" PRIuFAST16 " times.\n", info->monkeyIndex, info->inspectionCount);
        
        uint_fast32_t count = info->inspectionCount;
        if (count > secondHighestInspectionCount)
        {
            if (count > highestInspectionCount)
            {
                secondHighestInspectionCount = highestInspectionCount;
                highestInspectionCount = count;
            }
            else
            {
                secondHighestInspectionCount = count;
            }
        }
    }
    
    fclose(fp);
    
    printf("Part 1: Number of squares visited by the first knot:% *s%zu\n", 1, "", highestInspectionCount * secondHighestInspectionCount);
    //printf("Part 2: Number of squares visited by the first knot:% *s%zu\n", 1, "", signalStrength); 
}
