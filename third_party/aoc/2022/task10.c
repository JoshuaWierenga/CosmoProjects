// clang-format off
#include "libc/inttypes.h"
#include "libc/fmt/conv.h"
#include "libc/runtime/runtime.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "third_party/aoc/utilities/aocdatamanagement.h"
#include "third_party/aoc/utilities/macros.h"

#define showFullOutput false

#define addxInst "addx"
#define noopInst "noop"

#define crtWidth 40
#define crtHeight 6

enum Instruction
{
    addx,
    addxP2,
    noop,
};

int lenHelper(unsigned x) {
    if (x >= 1000000000) return 10;
    if (x >= 100000000)  return 9;
    if (x >= 10000000)   return 8;
    if (x >= 1000000)    return 7;
    if (x >= 100000)     return 6;
    if (x >= 10000)      return 5;
    if (x >= 1000)       return 4;
    if (x >= 100)        return 3;
    if (x >= 10)         return 2;
    return 1;
}

void ShowSpritePosition(uint_fast16_t registerX)
{
    printf("Sprite position: ");
    
    for (uint_fast8_t i = 0; i < crtWidth; i++)
    {
        if (abs(registerX - i) <= 1)
        {
            putchar('#');
        }
        else
        {
            putchar('.');
        }
    }
    
    puts("\n");
}

void PerformCycle(uint_fast16_t *ip, char* crtScreen, enum Instruction instr, int_fast8_t value, uint_fast16_t* registerX)
{
    //CRT display
    uint_fast8_t crtRow = *ip / 40;
    uint_fast8_t crtColumn = *ip % 40;
    
    //printf("%i, %i\n", *registerX, crtColumn);
    if (abs(*registerX - crtColumn) <= 1)
    {
        crtScreen[crtRow * crtWidth + crtColumn] = '#';
    }
    else
    {
        crtScreen[crtRow * crtWidth + crtColumn] = '.';
    }
    
    // CPU
    if (instr == addxP2)
    {
        *registerX += value;
    }
    
    (*ip)++;
}

void PerformFullCycle(uint_fast16_t *ip, char* crtScreen, enum Instruction instr, int_fast8_t value, uint_fast16_t* registerX)
{
    size_t ipDigits = lenHelper(*ip + 1);
    
    if (instr != addxP2)
    {
        printf("Start cycle% *s%i: begin executing ", MAX(2, 4 - ipDigits), "", *ip + 1);
    }
    
    switch (instr)
    {
        case addx:
        printf("addx %i\n", value);
        break;
        case noop:
        puts("noop");
        break;
        default:
        break;
    }
    
    uint_fast8_t crtRow = *ip / 40;
    uint_fast8_t crtColumn = *ip % 40;
    
    PerformCycle(ip, crtScreen, instr, value, registerX);
    
    printf("During cycle% *s%i: CRT draws pixel in position %i\n", MAX(1, 3 - ipDigits), "", *ip, crtColumn);
    printf("Current CRT row: %.*s\n", crtWidth, crtScreen + crtRow * crtWidth);
    
    if (instr != addx)
    {
        printf("End of cycle% *s%i: finish executing ", MAX(1, 3 - ipDigits), "", *ip);
    }
    
    switch (instr)
    {
        case addxP2:
        printf("addx %i (Register X is now %i)\n", value, *registerX);
        ShowSpritePosition(*registerX);
        break;
        case noop:
        puts("noop");
        break;
        default:
        break;
    }
    
    putchar('\n');
}

//TODO Support performance testing
void task10()
{
    FILE* fp = faocdataopen(__FILE__);
    
    // Long enough for the longest input I can see which is "addx", a space, a hyphen, two digits, a new line and a terminating char
    char chunk[10];
    
    uint_fast16_t ip = 0;
    uint_fast16_t registerX = 1;
    bool continueAddx = false;
    
    char crtScreen[crtWidth * crtHeight + 1] = {0};
    
    uint_fast16_t signalStrength = 0;
    
#if showFullOutput
    ShowSpritePosition(registerX);
#endif
    
    while(fgets(chunk, sizeof(chunk), fp) != NULL)
    {
        // All instructions are 4 characters long
        char instructionString[5] = {0};
        memcpy(instructionString, chunk, 4 * sizeof *instructionString);
        
        enum Instruction instruction;
        int_fast8_t addxValue = 0;
        
        if (strcmp(instructionString, addxInst) == 0 && chunk[4] == ' ')
        {
            instruction = addx;
            
            char* integerEndPtr;
            addxValue = strtol(chunk + 5, &integerEndPtr, 10);
            if (integerEndPtr == chunk || integerEndPtr == NULL || (integerEndPtr[0] != '\n' && integerEndPtr[0] != '\0'))
            {
                perror("Unable to parse input file");
                exit(1);
            }
        }
        else if (strcmp(instructionString, noopInst) == 0)
        {
            instruction = noop;
        }
        else
        {
            perror("Unable to parse input file");
            exit(1);
        }
        
#if showFullOutput
        PerformFullCycle(&ip, crtScreen, instruction, addxValue, &registerX);
#else
        PerformCycle(&ip, crtScreen, instruction, addxValue, &registerX);
#endif  
        switch (ip)
        {
            case 20:
            case 60:
            case 100:
            case 140:
            case 180:
            case 220:
            printf("strength at ip %i * %i = %i\n", ip, registerX, ip * registerX);
            signalStrength += ip * registerX;
            break;
        }
        
        if (instruction == addx)
        {
#if showFullOutput
            PerformFullCycle(&ip, crtScreen, addxP2, addxValue, &registerX);
#else
            PerformCycle(&ip, crtScreen, addxP2, addxValue, &registerX);
#endif  
            
            switch (ip)
            {
                case 20:
                case 60:
                case 100:
                case 140:
                case 180:
                case 220:
                printf("strength at ip %i * %i = %i\n", ip, registerX, ip * registerX);
                signalStrength += ip * registerX;
                break;
            }
        }
    }
    
    fclose(fp);
    
    printf("Part 1: Number of squares visited by the first knot:% *s%zu\n", 1, "", signalStrength);
    puts("Part 2: CRT test:");
    printf("%.*s\n", crtWidth, crtScreen);
    printf("%.*s\n", crtWidth, crtScreen + 1 * crtWidth);
    printf("%.*s\n", crtWidth, crtScreen + 2 * crtWidth);
    printf("%.*s\n", crtWidth, crtScreen + 3 * crtWidth);
    printf("%.*s\n", crtWidth, crtScreen + 4 * crtWidth);
    printf("%.*s\n", crtWidth, crtScreen + 5 * crtWidth);
}
