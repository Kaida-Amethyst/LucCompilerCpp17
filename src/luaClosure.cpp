//
// Created by ziyue on 2021/6/14.
//

#include "luaClosure.h"
#include "luaValue.h"

#include <utility>

luaClosure::luaClosure(Prototype * p):proto{p},func{nullptr},upvals{nullptr} {
    auto nUpvals = p->Upvalues == nullptr ? 0 : p->Upvalues->size();
    if (nUpvals > 0){
        this->upvals = new std::vector<luaValue*>(nUpvals);
    }
}

luaClosure::luaClosure(ExFunction f, int nUpvals):proto{nullptr}, func{std::move(f)}, upvals{nullptr} {
    if (nUpvals > 0){
        this->upvals = new std::vector<luaValue*>(nUpvals);
    }
}