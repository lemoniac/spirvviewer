#ifndef TYPE_H
#define TYPE_H

#include <iostream>
#include <map>

typedef unsigned Id;

struct Type {
    enum Types {
        Void, Bool, Int, Float, Vector, Matrix, Array, Pointer
    };

    Type(Types type, unsigned width = 1, bool signedness = true): type(type), width(width), signedness(signedness) { }

    Type() { }

    Types type = Bool;

    unsigned width = 1;
    bool signedness = true;

    uint32_t nested_type;
};

std::ostream &operator<<(std::ostream &os, const Type &type);

extern std::map<uint32_t, Type> types;

#endif//TYPE_H
