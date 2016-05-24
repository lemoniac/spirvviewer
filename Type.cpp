#include "Type.h"

std::map<uint32_t, Type> types;

std::ostream &operator<<(std::ostream &os, const Type &type)
{
    os << std::dec;
    switch(type.type)
    {
        case Type::Void: os << "Void"; break;
        case Type::Bool: os << "Bool"; break;
        case Type::Int: os << "Int" << type.width; break;
        case Type::Float: os << "Float" << type.width; break;
        case Type::Vector: os << "Vector" << type.width << "_" << types.at(type.nested_type); break;
        case Type::Matrix: os << "Matrix" << type.width << "_" << types.at(type.nested_type); break;
        case Type::Array: os << "Array" << type.width << "_" << types.at(type.nested_type); break;
        case Type::Pointer: os << "Pointer_" << types.at(type.nested_type); break;
        default: os << "Unknown Type";
    }

    os << std::hex;

    return os;
}