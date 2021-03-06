#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <stdexcept>
#include <map>
#include <vector>
#include "spirv.h"

#include "Type.h"

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

const std::vector<std::string> function_controls = {
    "None",
    "Inline",
    "DontInline",
    "Pure",
    "Const"
};

const std::vector<std::string> selection_controls = {
    "None",
    "Flatten",
    "DontFlatten"
};

unsigned identation = 0;
void incIdent() { identation++; }
void decIdent() { if(identation == 0) throw std::runtime_error("???"); identation--; }
std::string ident()
{
    std::string res = "";
    for(unsigned i = 0; i < identation; i++) res+= "    ";
    return res;
}

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

    void decodeSource() // 3
    {
        uint32_t source_language = decodeId();
        uint32_t version = decodeId();
        std::cout << ident() << "Source " << source_language << " " << version;
        if(offset + 1 < length)
        {
            uint32_t file = decodeId();
            std::cout << " " << file;
        }

        if(offset + 1 < length)
        {
            std::string source = decodeLiteralString();
            std::cout << " " << source;
        }

        std::cout << std::endl;
    }

    void decodeName() // 5
    {
        uint32_t target = decodeId();
        std::string name = decodeLiteralString();

        names[target] = name;

        std::cout << ident() << "Name " << std::hex << target << " " << name << std::endl;
    }

    void decodeExtInstImport() // 11
    {
        uint32_t result = decodeId();
        std::string name = decodeLiteralString();

        std::cout << ident() << "ExtInstImport " << result << " " << name << std::endl;
    }

    void decodeExtInst() // 12
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t set = decodeId();
        uint32_t instruction = decodeId();

        std::cout << ident() << "ExtInst " << types.at(resultType) << " " << result << " " << set << " " << instruction;

        while(offset + 1 < length)
        {
            uint32_t operand = decodeId();
            std::cout << " " << operand;
        }

        std::cout << std::endl;
    }

    void decodeMemoryModel() // 14
    {
        uint32_t addressingModel = decodeId();
        uint32_t memoryModel = decodeId();

        std::cout << ident() << "MemoryModel " << addressing_models[addressingModel] << " " << memory_models[memoryModel] << std::endl;
    }

    void decodeEntryPoint() // 15
    {
        uint32_t executionModel = decodeId();
        uint32_t entryPoint = decodeId();
        std::string name = decodeLiteralString();

        std::cout << ident() << "EntryPoint " << execution_models.at(executionModel) << " " << entryPoint << " " << name;

        while(offset + 1 < length)
        {
            uint32_t interface = decodeId();
            std::cout << " " << interface;
        }

        std::cout << std::endl;
    }

    void decodeExecutionMode() // 16
    {
        uint32_t entry_point = decodeId();
        uint32_t mode = decodeId();
        std::cout << ident() << "ExecutionMode " << entry_point << " " << mode;

        while(offset + 1 < length)
        {
            uint32_t optional = decodeId();
            std::cout << " " << optional;
        }

        std::cout << std::endl;
    }

    void decodeCapability() // 17
    {
        uint32_t capability = decodeId();
        std::cout << ident() << "Capability " << capabilities[capability] << std::endl;
    }

    void decodeTypeVoid() // 19
    {
        uint32_t result = decodeId();
        types[result] = Type{Type::Void};
        std::cout << ident() << "TypeVoid " << result << std::endl;
    }

    void decodeTypeBool() // 20
    {
        uint32_t result = decodeId();
        types[result] = Type{Type::Bool};
        std::cout << ident() << "TypeBool " << result << std::endl;
    }

    void decodeTypeInt() // 21
    {
        uint32_t result = decodeId();
        uint32_t width = decodeId();
        uint32_t signedness = decodeId();
        types[result] = Type{Type::Int, width, signedness != 0};
        std::cout << ident() << "TypeInt " << result << " " << width << " " << signedness << std::endl;
    }

    void decodeTypeFloat() // 22
    {
        uint32_t result = decodeId();
        uint32_t width = decodeId();
        types[result] = Type{Type::Float, width};
        std::cout << ident() << "TypeFloat " << result << " " << width << std::endl;
    }

    void decodeTypeVector() // 23
    {
        uint32_t result = decodeId();
        uint32_t component_type = decodeId();
        uint32_t component_count = decodeId();
        Type type{Type::Vector, component_count};
        type.nested_type = component_type;
        types[result] = type;
        std::cout << ident() << "TypeVector " << result << " " << component_type << " " << component_count << std::endl;
    }

    void decodeTypeMatrix() // 24
    {
        uint32_t result = decodeId();
        uint32_t column_type = decodeId();
        uint32_t column_count = decodeId();
        Type type{Type::Matrix, column_count};
        type.nested_type = column_type;
        types[result] = type;
        std::cout << "TypeMatrix " << result << " " << column_type << " " << column_count << std::endl;
    }

    void decodeTypeArray() // 28
    {
        uint32_t result = decodeId();
        uint32_t element_type = decodeId();
        uint32_t length = decodeId();
        Type type{Type::Array, length};
        type.nested_type = element_type;
        types[result] = type;
        std::cout << ident() << "TypeArray " << result << " " << element_type << " " << length << std::endl;
    }

    void decodeTypePointer() // 32
    {
        Type type{Type::Pointer};
        uint32_t result = decodeId();
        uint32_t storage_class = decodeId();
        type.nested_type = decodeId();
        types[result] = type;
        std::cout << ident() << "TypePointer " << result << " " << storage_classes[storage_class] << " " << type.nested_type << std::endl;
    }

    void decodeTypeFunction() // 33
    {
        uint32_t result = decodeId();
        uint32_t return_type = decodeId();
        std::cout << ident() << "TypeFunction " << result << " " << return_type << " (";
        while(offset + 1 < length)
        {
            uint32_t paremeter_type = decodeId();
            std::cout << " " << paremeter_type;
        }
        std::cout << ")" << std::endl;
    }

    void decodeConstantTrue() // 41
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        std::cout << "ConstantTrue "  << types.at(resultType) << " " << result << std::endl;
    }

    void decodeConstantFalse() // 42
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        std::cout << "ConstantFalse "  << types.at(resultType) << " " << result << std::endl;
    }

    void decodeConstant() // 43
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();

        std::cout << ident() << "Constant " << types.at(resultType) << " " << result;

        while(offset + 1 < length)
        {
            uint32_t value = decodeId();

            auto type = types.find(resultType);
            if(type != types.end())
            {
                switch(type->second.type)
                {
                    case Type::Float:
                    {
                        union {
                            uint32_t u;
                            float f;
                        } tmp;
                        tmp.u = value;
                        std::cout << " " << tmp.u << " " << tmp.f;
                        break;
                    }
                    default:
                        std::cout << " " << value;
                }
            }
        }

        std::cout << std::endl;
    }

    void decodeConstantComposite() // 44
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();

        std::cout << ident() << "ConstantComposite " << types.at(resultType) << " " << result;
        while(offset + 1 < length)
        {
            uint32_t constituent = decodeId();
            std::cout << " " << constituent;
        }

        std::cout << std::endl;
    }

    void decodeFunction() // 54
    {
        uint32_t result_type = decodeId();
        uint32_t result  = decodeId();
        uint32_t function_control = decodeId();
        uint32_t function_type = decodeId();

        std::cout << ident() << "Function " << types.at(result_type) << " " << names.at(result) << " " << function_controls[function_control] << " " << function_type << std::endl;

        incIdent();
    }

    void decodeFunctionParameter() // 55
    {
        uint32_t result_type = decodeId();
        uint32_t result  = decodeId();

        std::cout << ident() << "FunctionParameter " << types.at(result_type) << " " << names.at(result) << std::endl;
    }

    void decodeFunctionEnd() // 56
    {
        decIdent();
        std::cout << ident() << "FunctionEnd" << std::endl;
    }

    void decodeVariable() // 59
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t storage_class = decodeId();
        std::cout << ident() << "Variable " << types.at(resultType) << " " << names.at(result) << " " << storage_classes.at(storage_class);

        if(offset + 1 < length)
        {
            uint32_t initializer = decodeId();
            std::cout << " " << initializer;
        }

        std::cout << std::endl;
    }

    void decodeLoad() // 61
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t pointer = decodeId();

        std::cout << ident() << "Load " << types.at(resultType) << " " << result << " ";
        if(names.find(pointer) != names.end())
            std::cout << names.at(pointer);
        else
            std::cout << pointer;

        if(offset + 1 < length)
        {
            uint32_t memoryAccess = decodeId();
            std::cout << " " << memoryAccess;
        }

        std::cout << std::endl;
    }

    void decodeStore() // 62
    {
        uint32_t pointer = decodeId();
        uint32_t object = decodeId();

        std::cout << ident() << "Store ";
        if(names.find(pointer) != names.end())
            std::cout << names.at(pointer);
        else
            std::cout << pointer;
         std::cout << " " << object;

        if(offset + 1 < length)
        {
            uint32_t memoryAccess = decodeId();
            std::cout << " " << memoryAccess;
        }

        std::cout << std::endl;
    }

    void decodeAccessChain() // 65
    {
        uint32_t result_type = decodeId();
        uint32_t result = decodeId();
        uint32_t base = decodeId();

        std::cout << ident() << "AccessChain " << types.at(result_type) << " " << result << " " << base;

        while(offset + 1 < length)
        {
            uint32_t index = decodeId();
            std::cout << " " << index;
        }
        std::cout << std::endl;
    }

    void decodeCompositeConstruct() // 80
    {
        uint32_t result_type = decodeId();
        uint32_t result = decodeId();
        std::cout << ident() << "CompositeConstruct " << types.at(result_type) << " " << result;

        while(offset + 1 < length)
        {
            uint32_t constituent = decodeId();
            std::cout << " " << constituent;
        }
        std::cout << std::endl;
    }

    void decodeFNegate() // 127
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand = decodeId();

        std::cout << ident() << "FNegate " << types.at(resultType) << " - " << result << " <- " << operand <<std::endl;
    }

    void decodeIAdd() // 128
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "IAdd " << types.at(resultType) << " " << result << " <- " << operand1 << " + " << operand2 << std::endl;
    }

    void decodeFAdd() // 129
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "FAdd " << types.at(resultType) << " " << result << " <- " << operand1 << " + " << operand2 << std::endl;
    }

    void decodeFSub() // 131
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "FSub " << types.at(resultType) << " " << result << " <- " << operand1 << " - " << operand2 << std::endl;
    }

    void decodeFMul() // 133
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "FMul " << types.at(resultType) << " " << result << " <- " << operand1 << " * " << operand2 << std::endl;
    }

    void decodeFDiv() // 136
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "FDiv " << types.at(resultType) << " " << result << " <- " << operand1 << " / " << operand2 << std::endl;
    }

    void decodeVectorTimesScalar() // 142
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t vector = decodeId();
        uint32_t scalar = decodeId();

        std::cout << ident() << "VectorTimesScalar " << types.at(resultType) << " " << result << " <- " << vector << " * " << scalar << std::endl;
    }

    void decodeMatrixTimesVector() // 145
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t matrix = decodeId();
        uint32_t vector = decodeId();

        std::cout << ident() << "MatrixTimesVector " << types.at(resultType) << " " << result << " <- " << matrix << " " << vector << std::endl;
    }

    void decodeMatrixTimesMatrix() // 146
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t left_matrix = decodeId();
        uint32_t right_matrix = decodeId();

        std::cout << ident() << "MatrixTimesMatrix " << types.at(resultType) << " " << result << " <- " << left_matrix << " " << right_matrix << std::endl;
    }

    void decodeDot() // 148
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t vector1 = decodeId();
        uint32_t vector2 = decodeId();

        std::cout << ident() << "Dot " << types.at(resultType) << " " << result << " <- " << vector1 << " * " << vector2 << std::endl;
    }

    void decodeLogicalEqual() // 164
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "LogicalEqual " << types.at(resultType) << " " << result << " <- " << operand1 << " == " << operand2 << std::endl;
    }

    void decodeLogicalAnd() // 167
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "LogicalAnd " << types.at(resultType) << " " << result << " <- " << operand1 << " & " << operand2 << std::endl;
    }

    void decodeLogicalNot() // 168
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand = decodeId();

        std::cout << ident() << "LogicalNot " << types.at(resultType) << " " << result << " <- !" << operand << std::endl;
    }

    void decodeIEqual() // 170
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "IEqual " << types.at(resultType) << " " << result << " <- " << operand1 << " == " << operand2 << std::endl;
    }

    void decodeINotEqual() // 171
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "INotEqual " << types.at(resultType) << " " << result << " <- " << operand1 << " != " << operand2 << std::endl;
    }

    void decodeSGreaterThan() // 173
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "SGreaterThan " << types.at(resultType) << " " << result << " <- " << operand1 << " > " << operand2 << std::endl;
    }

    void decodeSLessThan() // 177
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "SLessThan " << types.at(resultType) << " " << result << " <- " << operand1 << " < " << operand2 << std::endl;
    }

    void decodeFOrdLessThan() // 184
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "FOrdLessThan " << types.at(resultType) << " " << result << " <- " << operand1 << " < " << operand2 << std::endl;
    }

    void decodeFOrdGreaterThan() // 186
    {
        uint32_t resultType = decodeId();
        uint32_t result = decodeId();
        uint32_t operand1 = decodeId();
        uint32_t operand2 = decodeId();

        std::cout << ident() << "FOrdGreaterThan " << types.at(resultType) << " " << result << " <- " << operand1 << " > " << operand2 << std::endl;
    }

    void decodeSelectionMerge() // 247
    {
        uint32_t merge_block = decodeId();
        uint32_t selection_control = decodeId();

        std::cout << ident() << "SelectionMerge " << merge_block << " " << selection_controls.at(selection_control) << std::endl;
    }

    void decodeLabel() // 248
    {
        uint32_t result = decodeId();

        std::cout << result << ":" << std::endl;
    }

    void decodeBranch() // 249
    {
        uint32_t target_label = decodeId();

        std::cout << ident() << "Branch " << target_label << std::endl;
    }

    void decodeBranchConditional() // 250
    {
        uint32_t conditional = decodeId();
        uint32_t true_label = decodeId();
        uint32_t false_label = decodeId();

        std::cout << ident() << "BranchConditional " << conditional << " " << true_label << " " << false_label;

        while(offset + 1 < length)
        {
            uint32_t branch_weight = decodeId();
            std::cout << " " << branch_weight;
        }

        std::cout << std::endl;
    }

    void decodeReturn() // 253
    {
        std::cout << ident() << "Return" << std::endl;
    }

    void decodeReturnValue() // 254
    {
        uint32_t value = decodeId();

        std::cout << ident() << "ReturnValue " << value << std::endl;
    }

    unsigned offset = 0;

    const unsigned length;
    const uint32_t *operands;
};

void output(const std::string &nmemonic, const Instruction &i)
{
    std::cout << ident() << nmemonic << " ";

    std::cout << std::dec;
    std::cout << i.opcode << " " << i.length << std::endl;

    std::cout << std::hex;
    const uint32_t *operands = &i.rest;
    for(unsigned j = 0; j < i.length - 1; j++)
    {
        std::cout << ident() << "   " << operands[j] << std::endl;
    }
}

void decode(const Instruction &i)
{
    InstructionDecoder decoder(i);

    switch(spv::Op(i.opcode))
    {
        case spv::Op::OpNop: output("Nop", i); break; // 0

        case spv::Op::OpSource: decoder.decodeSource(); break; // 3

        case spv::Op::OpName: decoder.decodeName(); break; // 5
        case spv::Op::OpMemberName: output("Member", i); break; // 6

        case spv::Op::OpExtInstImport: decoder.decodeExtInstImport(); break; // 11
        case spv::Op::OpExtInst: decoder.decodeExtInst(); break; // 12

        case spv::Op::OpMemoryModel: decoder.decodeMemoryModel(); break; // 14
        case spv::Op::OpEntryPoint: decoder.decodeEntryPoint(); break; // 15
        case spv::Op::OpExecutionMode: decoder.decodeExecutionMode(); break; // 16

        case spv::Op::OpCapability: decoder.decodeCapability(); break; // 17

        case spv::Op::OpTypeVoid: decoder.decodeTypeVoid(); break; // 19
        case spv::Op::OpTypeBool: decoder.decodeTypeBool(); break; // 20
        case spv::Op::OpTypeInt: decoder.decodeTypeInt(); break; // 21
        case spv::Op::OpTypeFloat: decoder.decodeTypeFloat(); break; // 22
        case spv::Op::OpTypeVector: decoder.decodeTypeVector(); break; // 23
        case spv::Op::OpTypeMatrix: decoder.decodeTypeMatrix(); break; // 24

        case spv::Op::OpTypeArray: decoder.decodeTypeArray(); break; // 28

        case spv::Op::OpTypeStruct: output("TypeStruct", i); break; // 30

        case spv::Op::OpTypePointer: decoder.decodeTypePointer(); break; // 32
        case spv::Op::OpTypeFunction: decoder.decodeTypeFunction(); break; // 33

        case spv::Op::OpConstantTrue: decoder.decodeConstantTrue(); break; // 41
        case spv::Op::OpConstantFalse: decoder.decodeConstantFalse(); break; // 42
        case spv::Op::OpConstant: decoder.decodeConstant(); break; // 43
        case spv::Op::OpConstantComposite: decoder.decodeConstantComposite(); break; // 44
        case spv::Op::OpConstantSampler: output("ConstantSampler", i); break; // 45
        case spv::Op::OpConstantNull: output("ConstantNull", i); break; // 46


        case spv::Op::OpFunction: decoder.decodeFunction(); break; // 54
        case spv::Op::OpFunctionParameter: decoder.decodeFunctionParameter(); break; // 55
        case spv::Op::OpFunctionEnd: decoder.decodeFunctionEnd(); break; // 56
        case spv::Op::OpFunctionCall: output("FunctionCall", i); break; // 57

        case spv::Op::OpVariable: decoder.decodeVariable(); break; // 59

        case spv::Op::OpLoad: decoder.decodeLoad(); break; // 61
        case spv::Op::OpStore: decoder.decodeStore(); break; // 62

        case spv::Op::OpAccessChain: decoder.decodeAccessChain(); break; // 65

        case spv::Op::OpDecorate: output("Decorate", i); break; // 71
        case spv::Op::OpMemberDecorate: output("MemberDecorate", i); break; // 72

        case spv::Op::OpVectorShuffle: output("VectorShuffle", i); break; // 79
        case spv::Op::OpCompositeConstruct: decoder.decodeCompositeConstruct(); break; // 80
        case spv::Op::OpCompositeExtract: output("CompositeExtract", i); break; // 80

        case spv::Op::OpFNegate: decoder.decodeFNegate(); break; // 127
        case spv::Op::OpIAdd: decoder.decodeIAdd(); break; // 128
        case spv::Op::OpFAdd: decoder.decodeFAdd(); break; // 129
        case spv::Op::OpISub: output("ISub", i); break; // 130
        case spv::Op::OpFSub: decoder.decodeFSub(); break; // 131
        case spv::Op::OpIMul: output("IMul", i); break; // 132
        case spv::Op::OpFMul: decoder.decodeFMul(); break; // 133

        case spv::Op::OpFDiv: decoder.decodeFDiv(); break; // 136

        case spv::Op::OpVectorTimesScalar: decoder.decodeVectorTimesScalar(); break; // 142

        case spv::Op::OpMatrixTimesVector: decoder.decodeMatrixTimesVector(); break; // 145
        case spv::Op::OpMatrixTimesMatrix: decoder.decodeMatrixTimesMatrix(); break; // 146

        case spv::Op::OpDot: decoder.decodeDot(); break; // 148

        case spv::Op::OpLogicalEqual: decoder.decodeLogicalEqual(); break; // 164
        case spv::Op::OpLogicalNotEqual: output("LogicalNotEqual", i); break; // 165
        case spv::Op::OpLogicalOr: output("LogicalOr", i); break; // 166
        case spv::Op::OpLogicalAnd: decoder.decodeLogicalAnd(); break; // 167
        case spv::Op::OpLogicalNot: decoder.decodeLogicalNot(); break; // 168

        case spv::Op::OpIEqual: decoder.decodeIEqual(); break; // 170
        case spv::Op::OpINotEqual: decoder.decodeINotEqual(); break; // 171

        case spv::Op::OpSGreaterThan: decoder.decodeSGreaterThan(); break; // 173

        case spv::Op::OpSLessThan: decoder.decodeSLessThan(); break; // 177

        case spv::Op::OpFOrdLessThan: decoder.decodeFOrdLessThan(); break; // 184

        case spv::Op::OpFOrdGreaterThan: decoder.decodeFOrdGreaterThan(); break; // 186

        case spv::Op::OpFOrdLessThanEqual: output("FOrdLessThanEqual", i); break; // 188

        case spv::Op::OpPhi: output("Phi", i); break; // 245
        case spv::Op::OpLoopMerge: output("LoopMerge", i); break; // 246
        case spv::Op::OpSelectionMerge: decoder.decodeSelectionMerge(); break; // 247
        case spv::Op::OpLabel: decoder.decodeLabel(); break; // 248
        case spv::Op::OpBranch: decoder.decodeBranch(); break; // 249
        case spv::Op::OpBranchConditional: decoder.decodeBranchConditional(); break; // 250
        case spv::Op::OpSwitch: output("Switch", i); break; // 251
        case spv::Op::OpKill: output("Kill", i); break; // 252
        case spv::Op::OpReturn: decoder.decodeReturn(); break; // 253
        case spv::Op::OpReturnValue: decoder.decodeReturnValue(); break; // 254

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

