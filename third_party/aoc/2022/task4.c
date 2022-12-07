// clang-format off
#include "libc/inttypes.h"
#include "libc/fmt/conv.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"

void task4()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Assuming that section ids are at most 2 digits, this an id, a hyphen, an id, a comma, an id, a hyphen, an id, a new line and the string terminator
    char chunk[13];
    
    uint_fast16_t assignmentFullOverlapCount = 0;
    uint_fast16_t assignmentAnyOverlapCount = 0;
    
    startTesting();
    
    rewind(fp);
    assignmentFullOverlapCount = assignmentAnyOverlapCount = 0;
    
    while(fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        char* integerStartPtr = chunk;
        char* integerEndPtr;
        
        uint_fast16_t firstAssignmentStart = strtoul(integerStartPtr, &integerEndPtr, 10);
        if (integerEndPtr == integerStartPtr || integerEndPtr == NULL) // No chars were read or at end of string
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        integerStartPtr = integerEndPtr + 1; // Account for -
        uint_fast16_t firstAssignmentEnd = strtoul(integerStartPtr, &integerEndPtr, 10);
        if (integerEndPtr == integerStartPtr || integerEndPtr == NULL) // No chars were read or at end of string
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        integerStartPtr = integerEndPtr + 1; // Account for ,
        uint_fast16_t secondAssignmentStart = strtoul(integerStartPtr, &integerEndPtr, 10);
        if (integerEndPtr == integerStartPtr || integerEndPtr == NULL) // No chars were read or at end of string
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        integerStartPtr = integerEndPtr + 1; // Account for -
        uint_fast16_t secondAssignmentEnd  = strtoul(integerStartPtr, NULL, 10);
        if (integerEndPtr == integerStartPtr || integerEndPtr == NULL) // No chars were read or at end of string
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
        int_fast32_t startDiff = secondAssignmentStart - firstAssignmentStart;
        int_fast32_t endDiff = secondAssignmentEnd - firstAssignmentEnd;
        int_fast32_t firstDiff = firstAssignmentStart - secondAssignmentEnd;
        int_fast32_t secondDiff = secondAssignmentStart - firstAssignmentEnd;
        assignmentFullOverlapCount += startDiff * endDiff <= 0;
        assignmentAnyOverlapCount += startDiff * firstDiff >= 0 || startDiff * secondDiff <= 0;
    }
    
    endTesting();
    fclose(fp);
    
    printf("Part 1: Number of pairs with full assignment overlap: %" PRIdFAST16 "\n", assignmentFullOverlapCount);
    printf("Part 2: Number of pairs with any assignment overlap:  %" PRIdFAST16 "\n", assignmentAnyOverlapCount);
}
