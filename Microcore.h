#ifndef MICROCORE_H
#define MICROCORE_H

#include <vector>
#include "State.h"
#include "Microcode.h"

class Microcore {
public:
    void execute();

//private:
    State state;

    std::vector<uint32_t> instructions;
};

#endif//MICROCODE_H
