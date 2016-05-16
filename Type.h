#ifndef TYPE_H
#define TYPE_H

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
};

#endif//TYPE_H
