//
// Created by ziyue on 2021/6/13.
//

#include "luaStack.h"
#include <iostream>
#include <algorithm>

luaStack::luaStack(int size):slots{new std::vector<luaValue>(size)}, top{0}, prev{nullptr}, closure{nullptr},pc{0} {}

luaStack::~luaStack() { delete slots;}

void luaStack::check(int n) {
    auto free{slots->size() - top};
    for(auto i=free;i<n;i++){slots->emplace_back(nil);};
}

void luaStack::push(const luaValue & val) {
    if (top == slots->size()){
        std::cerr << "Stack Overflow! Location: state/lua_stack" << std::endl;
        std::cerr << "top = " << top << " size = " << slots->size() << std::endl;
    }
    slots->at(top) = val;
    top++;
}

luaValue luaStack::pop() {
    if (top < 1){std::cerr << "Stack Underflow! Location: state/lua_stack" << std::endl;}
    top -- ;
    luaValue val = slots->at(top);
    slots->at(top) = nil;
    return std::move(val);
}

int luaStack::absIndex(int idx) const {
    if (idx >= 0) return idx;
    return idx + top + 1;
}

bool luaStack::isValid(int idx) const {
    idx = absIndex(idx);
    return idx > 0 && idx <= top;
}

luaValue& luaStack::get(int idx) const {
    idx = absIndex(idx);
    if (idx <= 0 && idx > top){
        std::cerr << "Invalid idx! Location: lua_stack.cpp, idx = " << idx << std::endl;
        abort();
    }
    return slots->at(idx-1);
}


void luaStack::set(int idx, const luaValue& val){
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