#ifndef XASM_INSTRUCTION_H
#define XASM_INSTRUCTION_H

#include "libc/inttypes.h"

//#define PARAMETER_ZERO_SIZE 4
//#define PARAMETER_REGISTER_SIZE 4
//#define PARAMETER_IMMEDIATE_SIZE 8
#define PARAMETER_IMMEDIATE_MAX_VALUE 0xFF
#define PARAMETER_IMMEDIATE_MAX_HEX_DIGITS 2

enum ParameterType
{
    Zero,     // 4 Bit
    Register, // 4 Bit
    Immediate // 8 Bit
};

struct Instruction
{
    char mnemonic[4];
    char opcode;
    uint_fast8_t requiredParameterCount;
    size_t totalParameterCount;
    enum ParameterType* parameters;
};

extern const size_t InstructionCount;
extern const struct Instruction Instructions[];

#define MAX_OPERATION_COUNT 256

struct Operation
{
    struct Instruction instruction;
    char arguments[3];
};

#endif //XASM_INSTRUCTION_H
