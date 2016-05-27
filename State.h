#ifndef STATE_H
#define STATE_H

#include <glm/vec4.hpp>

struct State {
    uint32_t PC = 0;

    uint32_t uscalar[256];
    float fscalar[256];
    glm::ivec4 uvector[256];
    glm::vec4 fvector[256];

    struct Flags {
        bool zero = false;
    } flags;
};

#endif//STATE_H
