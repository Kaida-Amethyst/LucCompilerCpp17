//
// Created by ziyue on 2021/6/14.
//

#ifndef LUACOMPILER_RENEW_LUAVM_H
#define LUACOMPILER_RENEW_LUAVM_H

#include "luaState.h"
#include "byte.h"

//luaValue luaNIL{nil};

class LuaVM : public luaState{
//private:
//    int pc;
//    Prototype *proto;
private:
    virtual bool __eq(const luaValue &a, const luaValue &b);
    virtual bool __lt(const luaValue &a, const luaValue &b);
    virtual bool __le(const luaValue &a, const luaValue &b);
    LuaType __getTable(const luaValue& t, const luaValue& k, bool raw);
    void __setTable(const luaValue& t, luaValue k, luaValue v, bool raw);;
public:
    using luaState::luaState;
//    LuaVM(int stackSize, Prototype *proto):stack{new luaStack(stackSize)},proto{proto}, pc{0}{};
    LuaVM& operator=(const LuaVM &) = delete;
    LuaVM(const LuaVM &) = delete;

    /* Return the current pc */
    int PC() const;

    /* Change pc, usually used in jmp instruction*/
    void AddPC(int n);

    /* get instruction according to the current pc,
     * and let pc move to the next instruction */
    uint32_t Fetch();

    /* Obtain constant value from Prototype */
    void GetConst(int idx);

    /* For OpArgK, if the the 9-th bit is 1, it means GetConst
     * if the 9-th is 0, it means PushValue.
     * */
    void GetRK(int rk);

    virtual void Arith(ArithOp op);
    virtual bool Compare(int idx1, int idx2, CompareOp op);
    virtual void Len(int idx);
    virtual void Concat(int n);
    LuaType GetTable(int idx);

    void callLuaClosure(int nArgs, int nResults, luaClosure * c);
    void callExClosure(int nArgs, int nResults, luaClosure * c);
    void runLuaClosure();
    int Load(const std::string & chunk, const std::string& chunkName);
    int Load(byte * chunk, const std::string& chunkName);
    void Call(int nArgs, int nResults);
    int RegisterCount();
    void LoadVararg(int n);
    void LoadProto(int idx);

    void CloseUpvalues(int a);

    virtual LuaType GetField(int idx, std::string k);
    virtual LuaType GetI(int idx, int64_t i);

    virtual void SetTable(int idx);

    virtual void SetField(int idx, const std::string& k);
    virtual void SetI(int  idx, int64_t i);

    bool GetMetatable(int idx);

    void SetMetatable(int idx);

    uint32_t RawLen(int idx);
    bool RawEqual(int idx1, int idx2);
    LuaType RawGet(int idx);

    void RawSet(int idx);

    LuaType RawGetI(int idx, int64_t i);

    void RawSetI(int idx, int64_t i);

    bool Next(int idx);
};

Prototype * Compile(const std::string & chunkName, const std::string & chunk);

#endif //LUACOMPILER_RENEW_LUAVM_H
