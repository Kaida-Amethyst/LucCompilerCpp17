//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_INSTRUCTION_H
#define LUACOMPILER_RENEW_INSTRUCTION_H

#include <cstdint>
#include <tuple>
#include <string>
#include "chunkStruct.h"
#include "byte.h"

#include "luaVM.h"

extern const int MAXARG_Bx;
extern const int MAXARG_sBx;
extern const int LFIELDS_PER_FLUSE;

class Instruction{
private:
    uint32_t proto_code;
public:
    explicit Instruction(uint32_t);
    Instruction(const Instruction &) = delete;
    Instruction & operator=(const Instruction &) = delete ;
    int Opcode();

    // Extract operator number
    std::tuple<int, int, int> ABC();
    std::pair<int, int> ABx();
    std::pair<int, int> AsBx();
    int Ax();

    // get information from opcode table
    std::string OpName();
    byte OpMode();
    byte BMode();
    byte CMode();
    void Execute(LuaVM & vm);
};


void move(Instruction & i, LuaVM & vm);
void jmp(Instruction & i, LuaVM & vm);
void loadNil(Instruction & i, LuaVM & vm);
void loadBool(Instruction & i, LuaVM & vm);
void loadK(Instruction & i, LuaVM & vm);
void loadKx(Instruction & i, LuaVM & vm);



void __binaryArith(Instruction & i, LuaVM & vm, ArithOp op);

void __unaryArith(Instruction & i, LuaVM & vm, ArithOp op);

void add (Instruction & i, LuaVM & vm);  // +
void sub (Instruction & i, LuaVM & vm);  // -
void mul (Instruction & i, LuaVM & vm);  // *
void mod (Instruction & i, LuaVM & vm); // %
void Pow (Instruction & i, LuaVM & vm); // ^
void Div (Instruction & i, LuaVM & vm); // /
void idiv(Instruction & i, LuaVM & vm); // //
void band(Instruction & i, LuaVM & vm); // &
void bor (Instruction & i, LuaVM & vm); // |
void bxor(Instruction & i, LuaVM & vm); // ~
void shl (Instruction & i, LuaVM & vm); // <<
void shr (Instruction & i, LuaVM & vm); // >>
void unm (Instruction & i, LuaVM & vm); // -
void bnot(Instruction & i, LuaVM & vm); // ~

void __len(Instruction & i, LuaVM & vm);

void concat(Instruction & i, LuaVM & vm);

void __comare(Instruction & i, LuaVM & vm, CompareOp op);

void eq(Instruction & i, LuaVM & vm); // ==
void lt(Instruction & i, LuaVM & vm); // <
void le(Instruction & i, LuaVM & vm); // <=


void Not(Instruction & i, LuaVM & vm);
void testSet(Instruction & i, LuaVM & vm);
void test(Instruction & i, LuaVM & vm);
void length(Instruction & i, LuaVM & vm);

void forPrep(Instruction & i, LuaVM & vm);
void forLoop(Instruction & i, LuaVM & vm);


void newTable(Instruction & i, LuaVM & vm);
void getTable(Instruction & i, LuaVM & vm);
void setTable(Instruction & i, LuaVM & vm);
void setList (Instruction & i, LuaVM & vm);

void closure(Instruction & i, LuaVM & vm);
void call(Instruction & i, LuaVM & vm);

void _return(Instruction & i, LuaVM & vm);
void vararg(Instruction & i, LuaVM & vm);
void tailCall(Instruction & i, LuaVM & vm);
void self(Instruction & i, LuaVM & vm);

int _pushFuncAndArgs(int a, int b, LuaVM & vm);
void _popResults(int a, int c, LuaVM & vm);
void _fixStack(int a, LuaVM & vm);



void getUpVal(Instruction & i, LuaVM & vm);
void setUpVal(Instruction & i, LuaVM & vm);
void getTabUp(Instruction & i, LuaVM & vm);
void setTabUp(Instruction & i, LuaVM & vm);




#endif //LUACOMPILER_RENEW_INSTRUCTION_H
