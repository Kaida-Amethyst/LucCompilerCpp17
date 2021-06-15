//
// Created by ziyue on 2021/6/14.
//

#ifndef LUACOMPILER_RENEW_LUACLOSURE_H
#define LUACOMPILER_RENEW_LUACLOSURE_H

#include "chunkStruct.h"
#include "exFunction.h"
#include <vector>

//struct upvalue;

class luaValue;

class luaClosure{
private:
    Prototype * proto;
    ExFunction func;
    std::vector<luaValue*> * upvals;
public:
    explicit luaClosure(Prototype* p);
    luaClosure(ExFunction f, int nUpvals);
    friend class luaStack;
    friend class luaState;
    friend class LuaVM;
    friend class luaValue;
};

#endif //LUACOMPILER_RENEW_LUACLOSURE_H
