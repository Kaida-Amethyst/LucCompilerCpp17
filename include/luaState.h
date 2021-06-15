//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_LUASTATE_H
#define LUACOMPILER_RENEW_LUASTATE_H

#include "luaStack.h"
#include "arith.h"
#include "chunkStruct.h"
#include "exFunction.h"
#include <memory>
#include "luaTable.h"

class luaState{
//protected:
public:
    luaTable * registry;
    luaStack * stack;
    Prototype *proto;
    ExFunction exfunc;

    int pc;
    luaValue __arith(luaValue &a, luaValue &b, Operator &op);
    bool __eq(const luaValue &a, const luaValue &b) const;
    bool __lt(const luaValue &a, const luaValue &b) const;
    bool __le(const luaValue &a, const luaValue &b) const;
    LuaType __getTable(const luaValue& t, const luaValue& k);
    void __setTable(const luaValue& t, luaValue k, luaValue v);

public:
    luaState();
//    luaState(int stackSize, Prototype *proto):stack{new luaStack(stackSize)},proto{proto}, pc{0}{};
    int GetTop() const;
    int AbsIndex(int idx) const;
    bool CheckStack(int n);
    void Pop(int n);
    void Copy(int fromIdx, int toIdx);
    void PushValue(int idx);
    void Replace(int idx);
    void Insert(int idx);
    void Remove(int idx);
    void Rotate(int idx, int n);
    void SetTop(int idx);
    static std::string TypeName(LuaType tp);
    LuaType Type(int idx)const;
    bool IsNone(int idx)const;
    bool IsNil(int idx)const;
    bool IsNoneOrNil(int idx)const;
    bool IsBoolean(int idx)const;
    bool IsInteger(int idx)const;
    bool IsNumber(int idx)const;
    bool IsString(int idx)const;
    bool ToBoolean(int idx) const;
    int64_t ToInteger(int idx) const;
    std::pair<int64_t, bool> ToIntegerX(int idx) const ;
    double ToNumber(int idx)const;
    std::pair<double, bool> ToNumberX(int idx) const;
    std::string ToString(int idx);
    std::pair<std::string, bool> ToStringX(int idx);
    void PushNil();
    void PushBoolean(bool b);
    void PushInteger(int64_t n);
    void PushNumber(double n);
    void PushString(const std::string& s);
    void Arith(ArithOp op);
    bool Compare(int idx1, int idx2, CompareOp op);
    void Len(int idx);
    void Concat(int n);
    void NewTable();
    void CreateTable(int nArr, int nRec);
    LuaType GetTable(int idx);

    LuaType GetField(int idx, std::string k);
    LuaType GetI(int idx, int64_t i);
    void SetTable(int idx);

    void SetField(int idx, const std::string& k);
    void SetI(int  idx, int64_t i);

    void pushLuaStack(luaStack * stack);
    void popLuaStack();

    void pushExFunction(ExFunction f);
    void pushExClosure(ExFunction f, int n);
    bool isExFunction(int idx);
    ExFunction ToExFunction(int idx);

    void PushGlobalTable();
    LuaType GetGlobal(const std::string & name);
    void SetGlobal(const std::string & name);
    void Register(const std::string & name, ExFunction f);


    friend class luaStack;
};



#endif //LUACOMPILER_RENEW_LUASTATE_H
