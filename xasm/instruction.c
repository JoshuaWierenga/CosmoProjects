#include "libc/macros.internal.h"

#include "third_party/xasm/instruction.h"

#define FORMAT(x) #x[sizeof(#x) - 2] - '0', ARRAYLEN(x), x

// Instruction formats
static const enum ParameterType fRII2[] = {Register, Immediate};          // |  op  |   reg   |        imm        | LDB, LOD, STR, BEZ, BGZ, JSR
static const enum ParameterType fRRR3[] = {Register, Register, Register}; // |  op  |   reg   |   reg   |   reg   | ADD, SUB, AND, XOR, SHL, SHR
static const enum ParameterType fR_R2[] = {Register, Zero, Register};     // |  op  |   reg   | ignored |   reg   | LDI, STI
static const enum ParameterType fR__1[] = {Register, Zero, Zero};         // |  op  |   reg   |      ignored      | JPI
static const enum ParameterType f___0[] = {Zero, Zero, Zero};             // |  op  |           ignored           | HLT

const struct Instruction Instructions[] =
{
    // ARITHMETIC and LOGICAL operations
    {"ADD", '1', FORMAT(fRRR3)}, // R[d] <- R[s] +  R[t]
    {"SUB", '2', FORMAT(fRRR3)}, // R[d] <- R[s] -  R[t]
    {"AND", '3', FORMAT(fRRR3)}, // R[d] <- R[s] &  R[t]
    {"XOR", '4', FORMAT(fRRR3)}, // R[d] <- R[s] ^  R[t]
    {"SHL", '5', FORMAT(fRRR3)}, // R[d] <- R[s] << R[t]
    {"SHR", '6', FORMAT(fRRR3)}, // R[d] <- R[s] >> R[t]
    // TRANSFER between registers and memory
    {"LDB", '7', FORMAT(fRII2)}, // R[d]    <- addr
    {"LOD", '8', FORMAT(fRII2)}, // R[d]    <- M[addr]
    {"STR", '9', FORMAT(fRII2)}, // M[addr] <- R[d]
    {"LDI", 'A', FORMAT(fR_R2)}, // R[d]    <- M[R[t]]
    {"STI", 'B', FORMAT(fR_R2)}, // M[R[t]] <- R[d]
    // CONTROL
    {"HLT", '0', FORMAT(f___0)}, // halt
    {"BEZ", 'C', FORMAT(fRII2)}, // if (R[d] == 0) PC <- addr
    {"BGZ", 'D', FORMAT(fRII2)}, // if (R[d] >  0) PC <- addr
    {"JPI", 'E', FORMAT(fR__1)}, // PC   <- R[d]
    {"JSR", 'F', FORMAT(fRII2)}, // R[d] <- PC; PC <- addr
};

const size_t InstructionCount = ARRAYLEN(Instructions);
