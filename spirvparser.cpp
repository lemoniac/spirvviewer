#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include "spirv.h"

struct Instruction {
    uint16_t opcode;
    uint16_t length;
    uint32_t rest;
};

struct Header {
    uint32_t magic;
    uint32_t version;
    uint32_t generator;
    uint32_t bound;
    uint32_t reserved;
    uint32_t first;
};

void decode(const Instruction &i)
{
    switch(spv::Op(i.opcode))
    {
        case spv::Op::OpSource: std::cout << "Source "; break; // 3

        case spv::Op::OpName: std::cout << "Name "; break; // 5

        case spv::Op::OpExtInstImport: std::cout << "ExtInstImport "; break; // 11
       
        case spv::Op::OpMemoryModel: std::cout << "MemoryModel "; break; // 14
        case spv::Op::OpEntryPoint: std::cout << "EntryPoint "; break; // 15
        case spv::Op::OpExecutionMode: std::cout << "ExecutionMode "; break; // 16

        case spv::Op::OpCapability: std::cout << "Capability "; break; // 17

        case spv::Op::OpTypeVoid: std::cout << "TypeVoid "; break; // 19
        case spv::Op::OpTypeBool: std::cout << "TypeBool "; break; // 20
        case spv::Op::OpTypeInt: std::cout << "TypeInt "; break; // 21
        case spv::Op::OpTypeFloat: std::cout << "TypeFloat "; break; // 22
        case spv::Op::OpTypeVector: std::cout << "TypeVector "; break; // 23

        case spv::Op::OpTypeArray: std::cout << "TypeArray "; break; // 28

        case spv::Op::OpTypePointer: std::cout << "TypePointer "; break; // 32
        case spv::Op::OpTypeFunction: std::cout << "TypeFunction "; break; // 33

        case spv::Op::OpConstant: std::cout << "Constant "; break; // 43

        case spv::Op::OpFunction: std::cout << "Function "; break; // 54
        case spv::Op::OpFunctionParameter: std::cout << "FunctionParameter "; break; // 55
        case spv::Op::OpFunctionEnd: std::cout << "FunctionEnd "; break; // 56

        case spv::Op::OpVariable: std::cout << "Variable "; break; // 59

        case spv::Op::OpLoad: std::cout << "Load "; break; // 61
        case spv::Op::OpStore: std::cout << "Store "; break; // 62

        case spv::Op::OpAccessChain: std::cout << "AccessChain "; break; // 65

        case spv::Op::OpLabel: std::cout << "Label "; break; // 248

        case spv::Op::OpReturn: std::cout << "Return "; break; // 253
    }

    std::cout << std::dec;
    std::cout << i.opcode << " " << i.length << std::endl;
    
    std::cout << std::hex;    
    const uint32_t *operands = &i.rest;
    for(unsigned j = 0; j < i.length - 1; j++)
        std::cout << "   " << operands[j] << std::endl;
}

int main(int argc, char **argv)
{
    FILE *f = fopen("dataOut.spirv", "rb");
    
    uint8_t buffer[64 * 1024];
    size_t length = fread(buffer, 1, 64 * 1024, f);
    fclose(f);
    
    std::cout << length << std::endl;
    
    if(length < 6 * 4)
    {
        std::cerr << "Couldn't read header." << std::endl;
        return -1;
    }

    Header &header = *(Header *)buffer;
    if(header.magic != spv::MagicNumber)
    {
        std::cerr << "Wrong magic number." << std::endl;
        return -1;
    }

    length = length / 4 - 5;
    uint32_t *instructions = &header.first;
    
    
    for(unsigned i = 0; i < length; i++)
    {
        Instruction &instruction = *(Instruction *)(instructions + i);
        decode(instruction);
        i+= instruction.length - 1;
    }


    return 0;
}

