//
// Created by ziyue on 2021/6/13.
//

#include "luaStack.h"
#include "luaState.h"
#include <iostream>
#include <algorithm>

luaStack::luaStack(int size):
slots{new std::vector<luaValue>(size)},
top{0}, prev{nullptr},
closure{nullptr},
pc{0},state{nullptr},openuvs{nullptr} {}

luaStack::luaStack(int size, luaState * s):
slots{new std::vector<luaValue>(size)},
top{0}, prev{nullptr},
closure{nullptr},
pc{0},state{s},openuvs{nullptr} {}

luaStack::~luaStack() { delete slots;}

void luaStack::check(int n) {
    auto free{slots->size() - top};
    for(auto i=free;i<n;i++){slots->emplace_back(nil);};
}

void luaStack::push(const luaValue & val) {
    if (this->top == slots->size()){
        std::cerr << "Stack Overflow! Location: state/lua_stack" << std::endl;
        std::cerr << "top = " << this->top << " size = " << slots->size() << std::endl;
    }
    slots->at(top) = val;
    this->top++;
    return ;
}

luaValue luaStack::pop() {
    if (top < 1){std::cerr << "Stack Underflow! Location: state/lua_stack" << std::endl;}
    top -- ;
    luaValue val = slots->at(top);
    slots->at(top) = nil;
    return std::move(val);
}

int luaStack::absIndex(int idx) const {
    if (idx <= LUA_REGISTRYINDEX){
        return idx;
    }

    if (idx >= 0) return idx;
    return idx + top + 1;
}

bool luaStack::isValid(int idx) const {
    if (idx < LUA_REGISTRYINDEX){
        auto uvIdx = LUA_REGISTRYINDEX - idx - 1;
        auto c = this->closure;
        return c != nullptr && c->upvals != nullptr && uvIdx < c->upvals->size();
    }

    if (idx == LUA_REGISTRYINDEX){
        return true;
    }

    idx = absIndex(idx);
    return idx > 0 && idx <= top;
}

luaValue luaStack::get(int idx) const {
    if (idx < LUA_REGISTRYINDEX){
        auto uvIdx = LUA_REGISTRYINDEX - idx -1;
        auto c = this->closure;
        if (c == nullptr || c->upvals == nullptr || uvIdx >= c->upvals->size()){
            return luaValue(nil);
        }
        return *c->upvals->at(uvIdx);
    }

    if (idx == LUA_REGISTRYINDEX){
        return luaValue(this->state->registry);
    }

    idx = absIndex(idx);
    if (idx <= 0 && idx > top){
        std::cerr << "Invalid idx! Location: lua_stack.cpp, idx = " << idx << std::endl;
        abort();
    }
    return slots->at(idx-1);
}


void luaStack::set(int idx, const luaValue& val){
    if (idx < LUA_REGISTRYINDEX){
        auto uvIdx = LUA_REGISTRYINDEX - idx - 1;
        auto c = this->closure;
        if (c != nullptr && c->upvals != nullptr && uvIdx < c->upvals->size()){
            *(c->upvals->at(uvIdx)) = val;
        }
        return ;
    }

    if (idx == LUA_REGISTRYINDEX){
        if (val.type() == luaValue::Table){
            std::cerr << "set error!, val is not a table! Location: luaStack::set" << std::endl;
            abort();
        }
        this->state->registry = val.get<luaValue::Table>();
        return ;
    }

    idx = absIndex(idx);
    if (idx > 0 && idx <= top){
        slots->at(idx-1) = val;
        return;
    }else{
        std::cerr << "Invalid index!" << std::endl;
    }
}

void luaStack::reverse(int from, int to) {
    while (from < to){
        std::swap(slots->at(from), slots->at(to));
        from++;
        to--;
    }
}

std::vector<luaValue> * luaStack::popN(int n) {
    auto vals = new std::vector<luaValue>();
    for(int i=n-1;i>=0;i--){
        vals->insert(vals->begin(), this->pop());
    }
    return vals;
}

void luaStack::pushN(std::vector<luaValue> * vals, int n) {
    auto nVals = vals->size();
    n = n < 0?static_cast<int>(nVals):n;
    for(int i=0;i < n;i++){
        if (i < nVals){
            this->push(vals->at(i));
        }else{
            this->push(luaValue(nil));
        }
    }
}