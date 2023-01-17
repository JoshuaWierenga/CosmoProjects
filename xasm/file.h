#ifndef XASM_FILE_H
#define XASM_FILE_H

#include "third_party/xasm/instruction.h"

bool ParseFile(char assemblyFilePath[], uint_fast8_t* operationCount, struct Operation operations[]);

bool OutputBinary(char binaryFilePath[], uint_fast8_t operationCount, struct Operation operations[]);

#endif //XASM_FILE_H
