// clang-format off
#include "libc/inttypes.h"
#include "libc/fmt/conv.h"
#include "libc/mem/gc.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "libc/x/xasprintf.h"
#include "third_party/aoc/utilities/hashmap/src/cmap.h"

char* year = "2022";
char* inputFileName = "task4.data";

void task4()
{
    char* comPath = GetProgramExecutableName();
    dirname(comPath);
    
    char* comPathOLocation = strstr(comPath, "/o/");
    size_t comPathPrefixLength = comPathOLocation - comPath;
    size_t comPathSuffixLength = strlen(comPath) - comPathPrefixLength - 2;
    
    char* inputPath = _gc(xasprintf("%.*s%.*s/%s/%s", comPathPrefixLength, comPath, comPathSuffixLength, comPath + comPathPrefixLength + 2, year, inputFileName));
    
    // Based on top example from https://solarianprogrammer.com/2019/04/03/c-programming-read-file-lines-fgets-getline-implement-portable-getline/
    // as the issues presented are not relevant here
    FILE* fp = fopen(inputPath, "r");
    if (fp == NULL)
    {
        perror("Unable to open input file");
        exit(1);
    }
    
    // Assuming that section ids are at most 2 digtis, this an id, a hyphen, an id, a comma, an id, a hyphen, an id, a new line and the string terminator
    char chunk[13];
    
    uint_fast16_t assignmentFullOverlapCount = 0;
    uint_fast16_t assignmentAnyOverlapCount = 0;
    
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
    
    printf("Part 1: Number of pairs with full assignment overlap: %" PRIdFAST16 "\n", assignmentFullOverlapCount);
    printf("Part 2: Number of pairs with any assignment overlap:  %" PRIdFAST16 "\n", assignmentAnyOverlapCount);
    
    fclose(fp);
}
