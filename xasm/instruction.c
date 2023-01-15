#include "libc/macros.internal.h"

#include "third_party/xasm/instruction.h"

#define FORMAT(x) ARRAYLEN(x), x

// Instruction formats
static const enum ArgumentType fRII[] = {Register, Immediate};          // |  op  |   reg   |        imm        | LDB, LOD, STR, BEZ, BGZ, JSR
static const enum ArgumentType fRRR[] = {Register, Register, Register}; // |  op  |   reg   |   reg   |   reg   | ADD, SUB, AND, XOR, SHL, SHR
static const enum ArgumentType fR_R[] = {Register, Zero, Register};     // |  op  |   reg   | ignored |   reg   | LDI, STI
static const enum ArgumentType fR__[] = {Register, Zero, Zero};         // |  op  |   reg   |      ignored      | JPI
static const enum ArgumentType f___[] = {Zero, Zero, Zero};             // |  op  |           ignored           | HLT

const struct Instruction Instructions[] =
{
    // ARITHMETIC and LOGICAL operations
    {"ADD", '1', FORMAT(fRRR)}, // R[d] <- R[s] +  R[t]
    {"SUB", '2', FORMAT(fRRR)}, // R[d] <- R[s] -  R[t]
    {"AND", '3', FORMAT(fRRR)}, // R[d] <- R[s] &  R[t]
    {"XOR", '4', FORMAT(fRRR)}, // R[d] <- R[s] ^  R[t]
    {"SHL", '5', FORMAT(fRRR)}, // R[d] <- R[s] << R[t]
    {"SHR", '6', FORMAT(fRRR)}, // R[d] <- R[s] >> R[t]
    // TRANSFER between registers and memory
    {"LDB", '7', FORMAT(fRII)}, // R[d]    <- addr
    {"LOD", '8', FORMAT(fRII)}, // R[d]    <- M[addr]
    {"STR", '9', FORMAT(fRII)}, // M[addr] <- R[d]
    {"LDI", 'A', FORMAT(fR_R)}, // R[d]    <- M[R[t]]
    {"STI", 'B', FORMAT(fR_R)}, // M[R[t]] <- R[d]
    // CONTROL
    {"HLT", '0', FORMAT(f___)}, // halt
    {"BEZ", 'C', FORMAT(fRII)}, // if (R[d] == 0) PC <- addr
    {"BGZ", 'D', FORMAT(fRII)}, // if (R[d] >  0) PC <- addr
    {"JPI", 'E', FORMAT(fR__)}, // PC   <- R[d]
    {"JSR", 'F', FORMAT(fRII)}, // R[d] <- PC; PC <- addr
};

const size_t InstructionCount = ARRAYLEN(Instructions);
