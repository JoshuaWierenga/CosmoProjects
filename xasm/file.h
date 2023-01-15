#ifndef XASM_FILE_H
#define XASM_FILE_H

#include "third_party/xasm/instruction.h"

bool ParseFile(char* assemblyFilePath, struct Operation* operations[]);

#endif //XASM_FILE_H
