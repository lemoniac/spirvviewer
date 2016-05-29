#ifndef MICROCODE_H
#define MICROCODE_H

const uint8_t OP_KIND_MASK = 0b11110000;
const uint8_t OP_MASK = 0b00001111;
const uint8_t ALU_OP = 0;
const uint8_t ALU_OP_IMM = 1 << 4;

const uint8_t F_ALU_OP = 2 << 4;
const uint8_t F_ALU_OP_IMM = 3 << 4;


enum class AluOp : uint8_t {
    Add = 0 | ALU_OP,
    Sub = 1 | ALU_OP,
    Mul = 2 | ALU_OP,
    Div = 3 | ALU_OP,
    Mod = 4 | ALU_OP,
    LShift = 5 | ALU_OP,
    RShift = 6 | ALU_OP,
    // ...
};

enum class AluOpImm : uint8_t {
    Set = 0 | ALU_OP_IMM,
};

struct alu_op_t {
    AluOp op;
    uint8_t dst;
    uint8_t src0;
    uint8_t src1;

    uint32_t get() { return *(uint32_t *)this; }
};

struct alu_op_imm_t {
    AluOpImm op;
    uint8_t dst;
    uint16_t imm;

    uint32_t get() { return *(uint32_t *)this; }
};

#endif//MICROCODE_H
