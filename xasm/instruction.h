#ifndef XASM_INSTRUCTION_H
#define XASM_INSTRUCTION_H

#define ARGUMENT_ZERO_SIZE 4
#define ARGUMENT_REGISTER_SIZE 4
#define ARGUMENT_IMMEDIATE_SIZE 8

enum ArgumentType
{
    Zero,     // 4 Bit
    Register, // 4 Bit
    Immediate // 8 Bit
};

struct Instruction
{
    char* mnemonic;
    char opcode;
    size_t argumentCount;
    enum ArgumentType* argumentList;
};

extern const size_t InstructionCount;
extern const struct Instruction Instructions[];

#define MAX_OPERATION_COUNT 256

struct Operation
{
    struct Instruction instruction;
    char parameters[3];
};

#endif //XASM_INSTRUCTION_H
