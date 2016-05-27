#include <iostream>
#include "Microcore.h"

int main()
{
    Microcore ucore;

    ucore.instructions = std::vector<uint32_t>{
        alu_op_imm_t{AluOpImm::ISet, 1, 123}.get(),
        alu_op_imm_t{AluOpImm::ISet, 2, 321}.get(),
        alu_op_t{AluOp::IAdd, 0, 1, 2}.get()
    };

    ucore.execute();

    assert( ucore.state.uscalar[0] == 444 );

    return 0;
}
