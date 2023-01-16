#include "libc/inttypes.h"
#include "libc/mem/mem.h"
#include "libc/stdio/stdio.h"
#include "libc/str/str.h"
#include "libc/x/xgetline.h"

#include "third_party/xasm/file.h"
#include "third_party/xasm/instruction.h"

#define errorFormatString "Error: On line %" PRIuFAST8 " at column %" PRIuFAST8 ":"

bool ParseFile(char* assemblyFilePath, struct Operation* operations[])
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
    for(uint_fast8_t lineNumber = 0;
        lineNumber < MAX_OPERATION_COUNT && (line = _chomp(xgetline(fp))) != NULL;
        lineNumber++)
    {
        // Allow comments and empty lines for readability
        if (strncmp(line, "//", 2) == 0 || line[0] == '#' || line[0] == '\0')
        {
            continue;
        }

        size_t mnemonicNumber = 0;
        for (; mnemonicNumber < InstructionCount; mnemonicNumber++)
        {
            if (strncasecmp(line, Instructions[mnemonicNumber].mnemonic, 3) == 0 &&
                (line[3] == ' ' || line[3] == '\0'))
            {
                break;
            }
        }

        if (mnemonicNumber == InstructionCount)
        {
            fprintf(stderr, errorFormatString " Found invalid instruction mnemonic '%.*s'.\n",
                lineNumber + 1, 1, strcspn(line, " "), line);
            return false;
        }
        else
        {
            struct Instruction info = Instructions[mnemonicNumber];
            printf("Found Instruction: %s", info.mnemonic);
            for (uint_fast8_t i = 0; i < info.argumentCount; i++)
            {
                switch (info.argumentList[i])
                {
                    case Zero:
                        printf(" 0x0");
                        break;
                    case Register:
                        printf("  RX");
                        break;
                    case Immediate:
                        printf(" 0xX 0xX");
                        break;
                }
            }
            printf("\n");
        }

        free(line);
    }

    return true;
}
