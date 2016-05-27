#include <iostream>
#include "Microcore.h"

void Microcore::execute()
{
    while(state.PC < instructions.size())
    {
        uint32_t inst = instructions[state.PC];
        unsigned op_kind = inst & OP_KIND_MASK;
        unsigned op = inst & 0xFF;

        switch(op_kind)
        {
            case ALU_OP:
            {
                alu_op_t &alu_op = *(alu_op_t *)&inst;
                switch(AluOp(op))
                {
                    case AluOp::IAdd: state.uscalar[alu_op.dst] = state.uscalar[alu_op.src0] + state.uscalar[alu_op.src1]; break;
                }
                break;
            }

            case ALU_OP_IMM:
            {
                alu_op_imm_t &alu_op_imm = *(alu_op_imm_t *)&inst;
                switch(AluOpImm(op))
                {
                    case AluOpImm::ISet: state.uscalar[alu_op_imm.dst] = alu_op_imm.imm; break;
                    case AluOpImm::FSet: break;
                }
                break;
            }
        }

        state.PC++;
    }
}