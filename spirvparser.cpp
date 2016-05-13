#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdexcept>
#include <map>
#include <vector>
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

std::map<uint32_t, std::string> names;

const std::vector<std::string> storage_classes = {
    "UniformConstant",
    "Input",
    "Uniform",
    "Output",
    "Workgroup",
    "CrossWorkgroup",
    "Private",
    "Function",
    "Generic",
    "PushConstant",
    "AtomicCounter",
    "Image"};

const std::vector<std::string> execution_models = {
    "Vertex",
    "TessellationControl",
    "TesselationEvaluation",
    "Geometry",
    "Fragment",
    "GLCompute",
    "Kernel"};

const std::vector<std::string> capabilities = {
    "Matrix",
    "Shader",
    "Geometry",
    "Tesselation"
    // ...
};

const std::vector<std::string> addressing_models = {
    "Logical",
    "Physical32",
    "Physical64"
};

const std::vector<std::string> memory_models = {
    "Simple",
    "GLSL450",
    "OpenCL"
};

struct InstructionDecoder {
    InstructionDecoder(const Instruction &instruction): length(instruction.length), operands(&instruction.rest) { }

    uint32_t decodeId()
    {
        if(offset >= length)
            throw std::runtime_error("overflow");

        uint32_t id = operands[offset];
        offset++;
        return id;
    }

    std::string decodeLiteralString()
    {
        const char *start = (const char *)(operands + offset);
        unsigned length = 0;
        while(start[length++]);
        offset += (length / 4) + 1;
        return std::string(start, length - 1);
    }

    void decodeName()
    {
        uint32_t target = decodeId();
        std::string name = decodeLiteralString();

        names[target] = name;

        std::cout << "Name " << target << " " << name << std::endl;
    }

    void decodeExtInstImport()
    {
        uint32_t result = decodeId();
        std::string name = decodeLiteralString();

        std::cout << "ExtInstImport " << result << " " << name << std::endl;
    }

    void decodeMemoryModel() // 14
    {
        uint32_t addressingModel = decodeId();
        uint32_t memoryModel = decodeId();

        std::cout << "MemoryModel " << addressing_models[addressingModel] << " " << memory_models[memoryModel] << std::endl;
    }

    void decodeEntryPoint() // 15
    {
        uint32_t executionModel = decodeId();
        uint32_t entryPoint = decodeId();
        std::string name = decodeLiteralString();

        std::cout << "EntryPoint " << execution_models.at(executionModel) << " " << entryPoint << " " << name;

        while(offset + 1 < length)
        {
            uint32_t interface = decodeId();
            std::cout << " " << interface;
        }

        std::cout << std::endl;
    }

    void decodeCapability()
    {
        uint32_t capability = decodeId();
        std::cout << "Capability " << capabilities[capability] << std::endl;
    }

    void decodeLoad()
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t pointer = decodeId();

        std::cout << "Load " << resultType << " " << result << " " << pointer;

        if(offset + 1 < length)
        {
            uint32_t memoryAccess = decodeId();
            std::cout << " " << memoryAccess;
        }

        std::cout << std::endl;
    }

    void decodeStore()
    {
        uint32_t pointer = decodeId();
        uint32_t object = decodeId();

        std::cout << "Store " << pointer << " " << object;

        if(offset + 1 < length)
        {
            uint32_t memoryAccess = decodeId();
            std::cout << " " << memoryAccess;
        }

        std::cout << std::endl;
    }

    void decodeVariable()
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t storage_class = decodeId();

        std::cout << "Variable " << resultType << " " << result << " " << storage_classes.at(storage_class);

        if(offset + 1 < length)
        {
            uint32_t initializer = decodeId();
            std::cout << " " << initializer;
        }

        std::cout << std::endl;
    }

    void decodeConstant()
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();

        std::cout << "Constant " << resultType << " " << result;

        while(offset + 1 < length)
        {
            uint32_t constituent = decodeId();
            std::cout << " " << constituent;
        }

        std::cout << std::endl;
    }

    void decodeFAdd()
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << "FAdd " << resultType << " " << result << " " << operand1 << " " << operand2 << std::endl;
    }

    void decodeFSub()
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << "FSub " << resultType << " " << result << " " << operand1 << " " << operand2 << std::endl;
    }

    void decodeLabel()
    {
        uint32_t result = decodeId();

        std::cout << "Label " << result << std::endl;
    }

    unsigned offset = 0;

    const unsigned length;
    const uint32_t *operands;
};

void output(const std::string &nmemonic, const Instruction &i)
{
    std::cout << nmemonic << " ";

    std::cout << std::dec;
    std::cout << i.opcode << " " << i.length << std::endl;

    std::cout << std::hex;
    const uint32_t *operands = &i.rest;
    for(unsigned j = 0; j < i.length - 1; j++)
        std::cout << "   " << operands[j] << std::endl;
}

void decode(const Instruction &i)
{
    InstructionDecoder decoder(i);

    switch(spv::Op(i.opcode))
    {
        case spv::Op::OpNop: output("Nop", i); break; // 0

        case spv::Op::OpSource: output("Source", i); break; // 3

        case spv::Op::OpName: decoder.decodeName(); break; // 5
        case spv::Op::OpMemberName: output("Member", i); break; // 6

        case spv::Op::OpExtInstImport: decoder.decodeExtInstImport(); break; // 11
        case spv::Op::OpExtInst: output("ExtInst", i); break; // 12

        case spv::Op::OpMemoryModel: decoder.decodeMemoryModel(); break; // 14
        case spv::Op::OpEntryPoint: decoder.decodeEntryPoint(); break; // 15
        case spv::Op::OpExecutionMode: output("ExecutionMode", i); break; // 16

        case spv::Op::OpCapability: decoder.decodeCapability(); break; // 17

        case spv::Op::OpTypeVoid: output("TypeVoid", i); break; // 19
        case spv::Op::OpTypeBool: output("TypeBool", i); break; // 20
        case spv::Op::OpTypeInt: output("TypeInt", i); break; // 21
        case spv::Op::OpTypeFloat: output("TypeFloat", i); break; // 22
        case spv::Op::OpTypeVector: output("TypeVector", i); break; // 23
        case spv::Op::OpTypeMatrix: output("TypeMatrix", i); break; // 24

        case spv::Op::OpTypeArray: output("TypeArray", i); break; // 28

        case spv::Op::OpTypeStruct: output("TypeStruct", i); break; // 30

        case spv::Op::OpTypePointer: output("TypePointer", i); break; // 32
        case spv::Op::OpTypeFunction: output("TypeFunction", i); break; // 33

        case spv::Op::OpConstantTrue: output("ConstantTrue", i); break; // 41
        case spv::Op::OpConstantFalse: output("ConstantFalse", i); break; // 42
        case spv::Op::OpConstant: decoder.decodeConstant(); break; // 43
        case spv::Op::OpConstantComposite: output("ConstantComposite", i); break; // 44
        case spv::Op::OpConstantSampler: output("ConstantSampler", i); break; // 45
        case spv::Op::OpConstantNull: output("ConstantNull", i); break; // 46


        case spv::Op::OpFunction: output("Function", i); break; // 54
        case spv::Op::OpFunctionParameter: output("FunctionParameter", i); break; // 55
        case spv::Op::OpFunctionEnd: output("FunctionEnd", i); break; // 56
        case spv::Op::OpFunctionCall: output("FunctionCall", i); break; // 57

        case spv::Op::OpVariable: decoder.decodeVariable(); break; // 59

        case spv::Op::OpLoad: decoder.decodeLoad(); break; // 61
        case spv::Op::OpStore: decoder.decodeStore(); break; // 62

        case spv::Op::OpAccessChain: output("AccessChain", i); break; // 65

        case spv::Op::OpDecorate: output("Decorate", i); break; // 71
        case spv::Op::OpMemberDecorate: output("MemberDecorate", i); break; // 72

        case spv::Op::OpVectorShuffle: output("VectorShuffle", i); break; // 79
        case spv::Op::OpCompositeConstruct: output("CompositeConstruct", i); break; // 80
        case spv::Op::OpCompositeExtract: output("CompositeExtract", i); break; // 80

        case spv::Op::OpFNegate: output("OpFNegate", i); break; // 127
        case spv::Op::OpIAdd: output("IAdd", i); break; // 128
        case spv::Op::OpFAdd: decoder.decodeFAdd(); break; // 129
        case spv::Op::OpISub: output("ISub", i); break; // 130
        case spv::Op::OpFSub: decoder.decodeFSub(); break; // 131
        case spv::Op::OpIMul: output("IMul", i); break; // 132
        case spv::Op::OpFMul: output("FMul", i); break; // 133

        case spv::Op::OpFDiv: output("FDiv", i); break; // 136

        case spv::Op::OpVectorTimesScalar: output("VectorTimesScalar", i); break; // 136

        case spv::Op::OpMatrixTimesVector: output("MatrixTimesVector ", i); break; // 145
        case spv::Op::OpMatrixTimesMatrix: output("MatrixTimesMatrix ", i); break; // 146

        case spv::Op::OpDot: output("Dot", i); break; // 148

        case spv::Op::OpLogicalEqual: output("LogicalEqual", i); break; // 164

        case spv::Op::OpLogicalAnd: output("LogicalAnd", i); break; // 167
        case spv::Op::OpLogicalNot: output("LogicalNot", i); break; // 168

        case spv::Op::OpIEqual: output("IEqual", i); break; // 170
        case spv::Op::OpINotEqual: output("INotEqual", i); break; // 171

        case spv::Op::OpSGreaterThan: output("SGreaterThan", i); break; // 173

        case spv::Op::OpSLessThan: output("SLessThan", i); break; // 177

        case spv::Op::OpFOrdLessThan: output("FOrdLessThan", i); break; // 184

        case spv::Op::OpFOrdGreaterThan: output("FOrdGreaterThan", i); break; // 186

        case spv::Op::OpFOrdLessThanEqual: output("FOrdLessThanEqual", i); break; // 188

        case spv::Op::OpPhi: output("Phi", i); break; // 245
        case spv::Op::OpLoopMerge: output("LoopMerge", i); break; // 246
        case spv::Op::OpSelectionMerge: output("SelectionMerge", i); break; // 247
        case spv::Op::OpLabel: decoder.decodeLabel(); break; // 248
        case spv::Op::OpBranch: output("Branch", i); break; // 249
        case spv::Op::OpBranchConditional: output("BranchConditional", i); break; // 250
        case spv::Op::OpSwitch: output("Switch", i); break; // 251
        case spv::Op::OpKill: output("Kill", i); break; // 252
        case spv::Op::OpReturn: output("Return", i); break; // 253
        case spv::Op::OpReturnValue: output("ReturnValue", i); break; // 254

        default: output("UNKNOWN", i); break;
    }
}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");

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

