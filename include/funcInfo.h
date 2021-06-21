//
// Created by ziyue on 2021/6/17.
//

#ifndef CH17_FUNCINFO_H
#define CH17_FUNCINFO_H

#include <unordered_map>
#include "luaValue.h"
#include "block.h"
#include <vector>
#include "codeGen.h"

struct Prototype;

class locVarInfo{
public:
    locVarInfo * prev;
    std::string * name;
    int scopeLv;
    int slot;
    bool captured;
    locVarInfo(locVarInfo * p, const std::string & n, int scopelv, int sl, bool c);
};


class upvalInfo {
public:
    int locVarSlot;
    int upvalIndex;
    int index;
    upvalInfo(int ls, int ui, int ind);
};



class funcInfo{
private:
    std::unordered_map<luaValue, int> * constants;
    int usedRegs;
    int maxRegs;
    int scopeLv;
    std::vector<locVarInfo*> * locVars;
    std::unordered_map<std::string, locVarInfo*> * locNames;
    std::vector<std::vector<int> * > * breaks;
    funcInfo * parent;
    std::unordered_map<std::string, upvalInfo * > * upvalues;
    std::vector<uint32_t> * insts;
    std::vector<funcInfo *> * subFuncs;
    int numParams;
    bool isVararg;

public:
    friend void cgRetStat(funcInfo * fi, std::vector<Exp*> * exps);
    friend void cgForNumStat(funcInfo * fi, ForNumStat * stat);
    friend void cgForInStat(funcInfo * fi, ForInStat * stat);
    friend void cgLocalVarDeclStat(funcInfo * fi, LocalVarDeclStat * stat);
    friend void cgAssignStat(funcInfo * fi, AssignStat * stat);
    friend void cgVarargExp(funcInfo * fi, VarargExp * exp, int a, int n);
    friend void cgFuncDefExp(funcInfo * fi, FuncDefExp * exp, int a);
    friend void cgConcatExp(funcInfo * fi, ConcatExp * exp, int a);
    friend std::vector<luaValue> * getConstants(funcInfo * fi);
    friend std::vector<Upvalue> * getUpvalues(funcInfo * fi);
    friend Prototype * toProto(funcInfo * fi);
    friend std::vector<Prototype*> * toProtos(std::vector<funcInfo *> * fis);
    friend Prototype * GenProto(Block * chunk);
//    funcInfo();
    funcInfo(funcInfo * parent, FuncDefExp * fd);
    int indexOfConstant(const luaValue & k);
    int allocReg();
    int allocRegs(int n);
    void freeRegs(int n);
    void freeReg();
    void enterScope(bool breakable);
    int addLocVar(const std::string& name);
    int slotOfLocVar(const std::string & name);
    void removeLocVar(locVarInfo * locVar);
    void exitScope();

    void addBreakJmp(int pc);
    int getJmpArgA();
    int pc();
    void fixSbx(int pc, int sBx);

    int indexOfUpval(const std::string & name);

    void closeOpenUpvals();

    void emitABC(int a, int b, int c, int opcode);
    void emitABx(int a, int bx, int opcode);
    void emitAsBx(int a, int b, int opcode);
    void emitAx(int a, int opcode);

    void emitMove(int a, int b);
    void emitLoadNil(int a, int n);
    void emitLoadBool(int a, int b, int c);
    void emitLoadK(int a, const luaValue & k);
    void emitVararg(int a, int n);
    void emitClosure(int a, int bx);
    void emitNewTable(int a, int nArr, int nRec);
    void emitSetList(int a, int b, int c);
    void emitGetTable(int a, int b, int c);
    void emitSetTable(int a, int b, int c);
    void emitGetUpval(int a, int b);
    void emitSetUpval(int a, int b);
    void emitGetTabUp(int a, int b, int c);
    void emitSetTabUp(int a, int b, int c);
    void emitCall(int a, int nArgs, int nRet);
    void emitTailCall(int a, int nArgs);
    void emitReturn(int a, int n);
    void emitSelf(int a, int b, int c);
    int emitJmp(int a, int sBx);
    void emitTest(int a, int c);
    void emitTestSet(int a, int b, int c);
    int emitForPrep(int a, int sBx);
    int emitForLoop(int a, int sBx);
    void emitTForCall(int a, int c);
    void emitTForLoop(int a, int sBx);
    void emitUnaryOp(int op, int a, int b);
    void emitBinaryOp(int op, int a, int b, int c);
};

std::vector<luaValue> * getConstants(funcInfo * fi);
std::vector<Upvalue> * getUpvalues(funcInfo * fi);
Prototype * toProto(funcInfo * fi);
std::vector<Prototype*> * toProtos(std::vector<funcInfo *> * fis);

#endif //CH17_FUNCINFO_H
