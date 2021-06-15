//
// Created by ziyue on 2021/6/14.
//

#ifndef LUACOMPILER_RENEW_LUAVM_H
#define LUACOMPILER_RENEW_LUAVM_H

#include "luaState.h"
#include "byte.h"

class LuaVM : public luaState{
//private:
//    int pc;
//    Prototype *proto;
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


    void callLuaClosure(int nArgs, int nResults, luaClosure * c);
    void runLuaClosure();
    int Load(byte * chunk, const std::string& chunkName, char mode);
    void Call(int nArgs, int nResults);
    int RegisterCount();
    void LoadVararg(int n);
    void LoadProto(int idx);

};

#endif //LUACOMPILER_RENEW_LUAVM_H
