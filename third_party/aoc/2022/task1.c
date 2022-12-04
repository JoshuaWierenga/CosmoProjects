// clang-format off
#include "libc/inttypes.h"
#include "libc/limits.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "third_party/aoc/utilities/aocdatamanagement.c"

#define STRINGIFY(x) #x
#define LENGTH(x) (sizeof(STRINGIFY(x)) - 1)

char* year = "2022";
char* inputFileName = "task1.data";

void task1()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Cosmo does not have UINT_WIDTH so using a macro to determine it
    // Longest valid input is a a 32 bit integer, a new line and the string terminator
    char chunk[LENGTH(UINT_MAX) + 2];
    
    uint_fast32_t elfNum = 1;
    uint_fast32_t currentElfCalories = 0;
    uint_fast32_t thirdHighestElfCalories = 0;
    uint_fast32_t secondHighestElfCalories = 0;
    uint_fast32_t highestElfCalories = 0;
    
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
    
    printf("Part 1: Highest number of calories:              %" PRIdFAST32 "\n", highestElfCalories);
    printf("Part 2: Sum of three highest number of calories: %" PRIdFAST32 "\n", highestElfCalories + secondHighestElfCalories + thirdHighestElfCalories);
    
    fclose(fp);
}
