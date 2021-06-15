//
// Created by ziyue on 2021/6/14.
//

#ifndef LUACOMPILER_RENEW_LUACLOSURE_H
#define LUACOMPILER_RENEW_LUACLOSURE_H

#include "chunkStruct.h"

class luaClosure{
private:
    Prototype * proto;
public:
    explicit luaClosure(Prototype* p);
    friend class luaStack;
    friend class luaState;
    friend class LuaVM;
    friend class luaValue;
};

#endif //LUACOMPILER_RENEW_LUACLOSURE_H
