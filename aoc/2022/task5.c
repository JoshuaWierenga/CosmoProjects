// clang-format off
#include "libc/inttypes.h"
#include "libc/fmt/conv.h"
#include "libc/mem/gc.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "libc/x/x.h" // Yes xmalloc is somewhat a bad function
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"
#include "third_party/aoc/utilities/dynamicarray/ary.h"

void task5()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Given 9 stacks, each of size 3 with 1 extra between stacks we need 3 * 9, 1 * 8, a new line and the string terminator
    char chunk[37];
    
    // array or array of chars representing a ship with stacks of crates
    struct ary(struct ary_char*) crateStacksPart1;
    struct ary(struct ary_char*) crateStacksPart2;
    ary_init(&crateStacksPart1, 9);
    ary_init(&crateStacksPart2, 9);
    
    startTesting();
    
    rewind(fp);
    ary_clear(&crateStacksPart1);
    ary_clear(&crateStacksPart2);
    
    bool finishedInitialState = false;
    uint_fast8_t currentRow = 1;
    uint_fast8_t stackCount = 0;
        
    char* start;
    
    while(fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        // Stack Numbers and blank line after it
        if (chunk[1] == '1' || chunk[0] == '\n')
        {
            finishedInitialState = true;
            start = chunk + 1;
            continue;
        }
        
        // Initial Data
        if (!finishedInitialState)
        {
            currentRow++;
            
            char* currentCrate = chunk;
            while ((currentCrate = strchr(currentCrate, '[')) != NULL)
            {
                currentCrate++;
                uint_fast8_t currentStackIndex = (currentCrate - chunk) / 4;
                
                while (currentStackIndex >= stackCount)
                {
                    stackCount++;

                    struct ary_char* newStackPart1 = _gc(xmalloc(sizeof *newStackPart1));
                    struct ary_char* newStackPart2 = _gc(xmalloc(sizeof *newStackPart2));

                    // 8 is the longest I saw but this will dynamically expand anyway
                    ary_init(newStackPart1, 8);
                    ary_init(newStackPart2, 8);

                    ary_push(&crateStacksPart1, newStackPart1);
                    ary_push(&crateStacksPart2, newStackPart2);
                }
                
                struct ary_char* stackPart1 = crateStacksPart1.buf[currentStackIndex];
                struct ary_char* stackPart2 = crateStacksPart2.buf[currentStackIndex];

                ary_push(stackPart1, currentCrate[0]);
                ary_push(stackPart2, currentCrate[0]);
            }
            continue;
        }
        
        char* integerStartPtr = chunk;
        char* integerEndPtr;
        
        // Move instructions, all of form "Move xx from y to z\n"
        uint_fast8_t quantity = strtoul(chunk + 5, &integerEndPtr, 10);
        if (integerEndPtr == integerStartPtr || integerEndPtr == NULL) // No chars were read or at end of string
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        integerStartPtr = integerEndPtr + 6; // Account for " from "
        uint_fast8_t fromStackIndex = strtoul(integerStartPtr, &integerEndPtr, 10);
        if (integerEndPtr == integerStartPtr || integerEndPtr == NULL) // No chars were read or at end of string
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        integerStartPtr = integerEndPtr + 4; // Account for " to "
        uint_fast8_t toStackIndex = strtoul(integerStartPtr, &integerEndPtr, 10);
        if (integerEndPtr == integerStartPtr || integerEndPtr == NULL) // No chars were read or at end of string
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        struct ary_char* fromStackPart1 = crateStacksPart1.buf[fromStackIndex - 1];
        struct ary_char* toStackPart1 = crateStacksPart1.buf[toStackIndex - 1];
        struct ary_char* fromStackPart2 = crateStacksPart2.buf[fromStackIndex - 1];
        struct ary_char* toStackPart2 = crateStacksPart2.buf[toStackIndex - 1];

        // TODO Replace with slice into temp array, reverse and then
        // two splices just like part 2 except drawing from temp array?
        for (int i = 0; i < quantity; i++)
        {
            char crate = fromStackPart1->buf[0];
            ary_remove(fromStackPart1, 0);
            ary_insert(toStackPart1, 0, crate);
        }

        ary_splice(toStackPart2, 0, 0, fromStackPart2->buf, quantity);
        ary_splice(fromStackPart2, 0, quantity, NULL, 0);
    }
    
    endTesting();
    fclose(fp);
    
    fputs("Part 1: CrateMover 9000 Stack Tops: ", stdout);
    
    for (size_t i = 0; i < crateStacksPart1.len; i++)
    {
        struct ary_char* stack = crateStacksPart1.buf[i];
        putchar(stack->buf[0]);
        ary_release(stack);
    }
    
    fputs("\nPart 2: CrateMover 9001 Stack Tops: ", stdout);
    
    for (size_t i = 0; i < crateStacksPart2.len; i++)
    {
        struct ary_char* stack = crateStacksPart2.buf[i];
        putchar(stack->buf[0]);
        ary_release(stack);
    }

    putchar('\n');
    
    ary_release(&crateStacksPart1);
    ary_release(&crateStacksPart2);
}
