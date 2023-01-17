#include "libc/fmt/conv.h"
#include "libc/fmt/fmt.h"
#include "libc/inttypes.h"
#include "libc/mem/gc.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"

#include "third_party/xasm/file.h"
#include "third_party/xasm/instruction.h"

#define errorFormatString "Error: On line %" PRIuFAST8 " at column %zu: "

bool ParseMnemonic(char line[], size_t lineLength, uint_fast8_t lineNumber, size_t* mnemonicIndex)
{
    *mnemonicIndex = 0;

    for (; lineLength >= 3 && *mnemonicIndex < InstructionCount; (*mnemonicIndex)++)
    {
        if (strncasecmp(line, Instructions[*mnemonicIndex].mnemonic, 3) == 0 &&
            (line[3] == ' ' || line[3] == '\0'))
        {
            break;
        }
    }

    if (lineLength < 3 || *mnemonicIndex == InstructionCount)
    {
        fprintf(stderr, errorFormatString "Found invalid instruction mnemonic '%.*s'.\n",
                lineNumber + 1, 1, strcspn(line, " "), line);
        return false;
    }

    return true;
}

bool ParseImmediate(char line[], uint_fast8_t lineNumber, size_t* columnNumber, enum ParameterType parameter, char arguments[], size_t* argumentIndex, uint_fast8_t* parsedArgumentCount)
{
    switch (parameter)
    {
        case Zero:
            arguments[*argumentIndex] = '0';
            (*argumentIndex)++;
            break;
        case Register:
            if (line[*columnNumber] != 'R')
            {
                fprintf(stderr, errorFormatString "Found '%.*s' in place of expected register argument.\n",
                        lineNumber + 1, *columnNumber + 1, strcspn(line + *columnNumber, " ,"), line + *columnNumber);
                return false;
            }

            if (!isdigit(line[*columnNumber + 1]) && (line[*columnNumber + 1] < 'A' || line[*columnNumber + 1] > 'F'))
            {
                fprintf(stderr, errorFormatString "Register '%.*s' is not a valid register. Register number must be between 0 and F.\n",
                        lineNumber + 1, *columnNumber + 1, strcspn(line + *columnNumber, " ,"), line + *columnNumber);
                return false;
            }

            arguments[*argumentIndex] = line[*columnNumber + 1];

            (*argumentIndex)++;
            (*parsedArgumentCount)++;
            *columnNumber += 2;
            break;
        case Immediate: ;
            char* integerEndPtr;
            unsigned long immediate = strtoul(line + *columnNumber, &integerEndPtr, 0);
            if (integerEndPtr == NULL || integerEndPtr == line + *columnNumber || !isdigit(line[*columnNumber]))
            {
                fprintf(stderr, errorFormatString "Found '%.*s' in place of expected immediate argument.\n",
                        lineNumber + 1, *columnNumber + 1, strcspn(line + *columnNumber, " ,"), line + *columnNumber);
                return false;
            }

            if (immediate > PARAMETER_IMMEDIATE_MAX_VALUE)
            {
                fprintf(stderr, errorFormatString "Immediate 0x%lX is larger than maximum 0x%hhX.\n",
                        lineNumber + 1, *columnNumber + 1, immediate, PARAMETER_IMMEDIATE_MAX_VALUE);
                return false;
            }

            char hexImmediate[PARAMETER_IMMEDIATE_MAX_HEX_DIGITS + 1];
            sprintf(hexImmediate, "%lX", immediate);
            memcpy(arguments + *argumentIndex, hexImmediate, PARAMETER_IMMEDIATE_MAX_HEX_DIGITS);

            *argumentIndex += PARAMETER_IMMEDIATE_MAX_HEX_DIGITS;
            (*parsedArgumentCount)++;
            *columnNumber = integerEndPtr - line;
            break;
    }

    return true;
}

bool ParseFile(char* assemblyFilePath, struct Operation operations[])
{
    if (operations == NULL)
    {
        return false;
    }

    FILE* fp = fopen(assemblyFilePath, "r");
    if (fp == NULL)
    {
        return false;
    }
    // Ensure that fclose is recalled regardless of how this function returns
    _defer(fclose, fp);

    char* line;
    size_t lineLength;
    for(uint_fast8_t lineNumber = 0;
        lineNumber < MAX_OPERATION_COUNT && (line = _chomp(fgetln(fp, &lineLength))) != NULL;
        lineNumber++)
    {
        // fgetln returns the total returned number of bytes instead of a regular char count but since sizeof(char) = 1,
        // subtracting away the null pointer gives the correct total.
        lineLength--;

        // Allow comments and empty lines for readability
        if (strncmp(line, "//", 2) == 0 || line[0] == '#' || line[0] == '\0')
        {
            continue;
        }

        size_t mnemonicIndex;
        bool success = ParseMnemonic(line, lineLength, lineNumber, &mnemonicIndex);
        if (!success)
        {
            return false;
        }
        struct Instruction inst = operations->instruction = Instructions[mnemonicIndex];

        size_t parameterIndex = 0;
        size_t argumentIndex = 0;
        uint_fast8_t parsedArgumentCount = 0;
        size_t columnNumber = 4;
        while(parameterIndex < inst.totalParameterCount && columnNumber < lineLength)
        {
            success = ParseImmediate(line, lineNumber, &columnNumber, inst.parameters[parameterIndex], operations->arguments, &argumentIndex, &parsedArgumentCount);
            if (!success)
            {
                return false;
            }

            parameterIndex++;

            if (columnNumber >= lineLength || (inst.parameters[parameterIndex - 1] != Zero && line[columnNumber] != ','))
            {
                break;
            }

            columnNumber += strspn(line + columnNumber, " ,");
        }

        if (parsedArgumentCount < inst.requiredParameterCount)
        {
            char* preDigitString = parsedArgumentCount > 0 ? "only " : "";
            char* postDigitString = parsedArgumentCount == 1 ? "was " : "were ";

            fprintf(stderr, errorFormatString "Instruction '%s' requires %" PRIuFAST8 " arguments but %s%zu %sfound.\n",
                    lineNumber + 1, columnNumber + 1, inst.mnemonic, inst.requiredParameterCount, preDigitString,
                    parsedArgumentCount, postDigitString);
            return false;
        }

        if ((inst.requiredParameterCount == 0 && line[3] != '\0') ||
            (inst.requiredParameterCount > 0 && line[columnNumber] != '\0'))
        {
            fprintf(stderr, errorFormatString "More than %" PRIuFAST8 " arguments were found while parsing instruction '%s'.\n",
                    lineNumber + 1, 1, inst.requiredParameterCount, inst.mnemonic);
            return false;
        }

        printf("Found Instruction with %" PRIuFAST8 " arguments: %s", inst.requiredParameterCount, inst.mnemonic);
        for (size_t p = 0, a = 0; p < inst.requiredParameterCount; p++)
        {
            switch (inst.parameters[p])
            {
                case Zero:
                printf(" 0x0");
                break;
                case Register:
                printf("  R%c", operations->arguments[a]);
                a++;
                break;
                case Immediate:
                printf(" 0x%c 0x%c", operations->arguments[a], operations->arguments[a + 1]);
                a += 2;
                break;
            }
        }
        printf("\n");

        operations++;
    }

    return true;
}
