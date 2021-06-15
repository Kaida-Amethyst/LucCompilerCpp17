//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_LUASTACK_H
#define LUACOMPILER_RENEW_LUASTACK_H

#include <vector>
#include "luaValue.h"
#include "luaClosure.h"

class luaState;

class luaStack{
//private:
public:
    std::vector<luaValue> *slots;
    int top;
    luaStack * prev;
    luaClosure * closure;
    std::vector<luaValue> * varargs;
    int pc;
    luaState * state;
    std::unordered_map<int, luaValue*> * openuvs;
public:
    explicit luaStack(int size);
    luaStack(int size, luaState * s);
    ~luaStack();
    void check(int n); // Whether we can put n values into slots?
    void push(const luaValue& val);

    template<typename T>
    inline void emplace(const T & t){
        this->push(luaValue(t));
    }
    luaValue pop();
    int absIndex(int idx) const;
    bool isValid(int idx) const;
    luaValue get(int idx) const;
    void set(int idx, const luaValue& val);

    template<typename T>
    void set_v(int idx, const T & t){
        this->set(idx, luaValue(t));
    }
    void reverse(int from, int to);
    std::vector<luaValue> * popN(int n);
    void pushN(std::vector<luaValue> * vals, int n);
    friend class luaState;
    friend class LuaVM;
};

#endif //LUACOMPILER_RENEW_LUASTACK_H
