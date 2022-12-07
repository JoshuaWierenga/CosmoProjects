// clang-format off
#include "libc/inttypes.h"
#include "libc/limits.h"
#include "libc/fmt/conv.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"

void task1()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Cosmo does not have UINT_WIDTH so using a macro to determine it
    // Longest valid input is a a 32 bit integer, a new line and the string terminator
    char chunk[LENGTH(UINT_MAX) + 2];
    
    uint_fast32_t thirdHighestElfCalories = 0;
    uint_fast32_t secondHighestElfCalories = 0;
    uint_fast32_t highestElfCalories = 0;
    
    startTesting();
    
    rewind(fp);
    thirdHighestElfCalories = secondHighestElfCalories = highestElfCalories = 0;
    
    uint_fast32_t currentElfCalories = 0;
    uint_fast32_t elfNum = 1;
    
    while(fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        if (chunk[0] == '\n')
        {
            elfNum++;
            // There must be a better method with less branching
            // Initially I thought of using an array and then sorting it but thats lot of branching as well,
            // perhaps a non comparative sort?
            if (currentElfCalories > thirdHighestElfCalories)
            {
                if (currentElfCalories > secondHighestElfCalories)
                {
                    thirdHighestElfCalories = secondHighestElfCalories;
                    if (currentElfCalories > highestElfCalories)
                    {
                        secondHighestElfCalories = highestElfCalories;
                        highestElfCalories = currentElfCalories;
                    }
                    else
                    {
                        secondHighestElfCalories = currentElfCalories;
                    }
                }
                else
                {
                    thirdHighestElfCalories = currentElfCalories;
                }
            }
            currentElfCalories = 0;
            continue;
        }
        
        unsigned long input = strtoul(chunk, NULL, 10);
        if (chunk[0] == '-' || input == 0 || input >= UINT_MAX)
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        currentElfCalories += input;
    }
    
    endTesting();
    fclose(fp);
   
    printf("Part 1: Highest number of calories:              %" PRIuFAST32 "\n", highestElfCalories);
    printf("Part 2: Sum of three highest number of calories: %" PRIuFAST32 "\n", highestElfCalories + secondHighestElfCalories + thirdHighestElfCalories);
}
