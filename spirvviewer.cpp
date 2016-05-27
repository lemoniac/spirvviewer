#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include <glm/vec4.hpp>
#include <SDL.h>

#include "Microcore.h"
#include "Type.h"

struct Variable {
    Variable(): type(Type::Int) {}

    Type type;
    glm::vec4 f;
    glm::ivec4 i;
    glm::bvec4 b;
};

struct State {
    std::array<Variable, 32> variables;
    unsigned variables_size = 0;
    unsigned PC = 0;

    void setMaxVariables(unsigned max) { variables_size = max; }

    void checkType(Id id, Type type)
    {
        if(id >= variables_size)
            throw std::runtime_error("wrong variable id");
        if(variables[id].type != type)
            throw std::runtime_error("wrong type");
    }

    void addVariable(Id id, Type type)
    {
        variables[id].type = type;
    }

    void set(Id id, float value)
    {
        checkType(id, Type::Float);
        variables[id].f.x = value;
    }

    void set(Id id, int value)
    {
        checkType(id, Type::Int);
        variables[id].i.x = value;
    }

    void set(Id id, bool value)
    {
        checkType(id, Type::Bool);
        variables[id].b.x = value;
    }
};

struct Instruction {
    virtual void execute(State &state) = 0;

    virtual ~Instruction() { }
};

struct FAdd : public Instruction {
    FAdd(): Instruction() { }
    FAdd(Id result, Id op1, Id op2): Instruction(), result(result), op1(op1), op2(op2) { }

    void execute(State &state)
    {
        state.checkType(result, Type::Float);
        state.checkType(op1, Type::Float);
        state.checkType(op2, Type::Float);

        state.variables[result].f.x =
            state.variables[op1].f.x +
            state.variables[op2].f.x;
    }

    Id result;
    Id op1;
    Id op2;
};

struct Program {
    std::vector<std::unique_ptr<Instruction>> instructions;

    void execute(State &state)
    {
        for(const auto &i : instructions)
           i->execute(state);
    }
};

void init(State &state)
{
    state.setMaxVariables(3);
    state.addVariable(0, Type::Float);
    state.addVariable(1, Type::Float);
    state.addVariable(2, Type::Float);
}

void init(Program &program)
{
    program.instructions.emplace_back(new FAdd{0, 1, 2});
}

int main()
{
    std::cout << "glslviewer" << std::endl;

    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init Error" << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("glslviewer", 100, 100, 512, 512, SDL_WINDOW_SHOWN);
    if(win == nullptr)
    {
        std::cerr << "CreateWindow" << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surface = SDL_GetWindowSurface(win);
    SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);
    if(renderer == nullptr)
    {
        std::cerr << "CreateRenderer" << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0xFF, 0xFF);


    State state;
    init(state);
    Program program;
    init(program);

    unsigned counter = 0;

    while(1)
    {
        SDL_Event e;
        if(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                break;
        }

        SDL_SetRenderDrawColor(renderer, 0xFF, 0, counter & 0xFF, 0xFF);
        SDL_RenderClear(renderer);

        for(unsigned y = 0; y < 512; y++)
            for(unsigned x = 0; x < 512; x++)
            {
                State state;
                init(state);
                program.execute(state);
            }

        int *pixels = (int *)surface->pixels;
        for(unsigned i = 0; i < 100; i++)
            pixels[10000+i] = 0;

        SDL_UpdateWindowSurface(win);
        counter++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);

    SDL_Quit();

    return 0;
}

