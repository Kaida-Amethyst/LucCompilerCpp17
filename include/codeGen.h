//
// Created by ziyue on 2021/6/18.
//

#ifndef CH17_CODEGEN_H
#define CH17_CODEGEN_H

//#include "funcInfo.h"
#include "block.h"

class funcInfo;


struct Prototype ;

void cgBlock(funcInfo * fi, Block * node);
void cgRetStat(funcInfo * fi, std::vector<Exp*> * exps);
void cgStat(funcInfo * fi, Stat * node);

void cgFuncCallStat(funcInfo * fi, FuncCallStat * stat);
void cgBreakStat(funcInfo * fi, BreakStat * stat);
void cgDoStat(funcInfo * fi, DoStat * stat);
void cgWhileStat(funcInfo * fi, WhileStat * stat);
void cgRepeatStat(funcInfo * fi, RepeatStat * stat);
void cgIfStat(funcInfo * fi, IfStat * stat);
void cgForNumStat(funcInfo * fi, ForNumStat * stat);
void cgForInStat(funcInfo * fi, ForInStat * stat);
void cgLocalVarDeclStat(funcInfo * fi, LocalVarDeclStat * stat);
void cgAssignStat(funcInfo * fi, AssignStat * stat);
void cgLocalFuncDefStat(funcInfo * fi, LocalFuncDefStat * stat);


void cgExp(funcInfo * fi, Exp * node, int a, int n);
void cgVarargExp(funcInfo * fi, VarargExp * exp, int a, int n);
void cgFuncDefExp(funcInfo * fi, FuncDefExp * exp, int a);
void cgTableConstructorExp(funcInfo * fi, TableConstructorExp * exp, int a);
void cgUnopExp(funcInfo * fi, UnopExp * exp, int a);
void cgBinopExp(funcInfo * fi, BinopExp * exp, int a);
void cgConcatExp(funcInfo * fi, ConcatExp * exp, int a);
void cgNameExp(funcInfo * fi, NameExp * exp, int a);
void cgTableAccessExp(funcInfo * fi, TableAccessExp * exp, int a);
void cgFuncCallExp(funcInfo * fi, FuncCallExp * exp, int a, int n);
void cgTailCallExp(funcInfo * fi, FuncCallExp * exp, int a);

Prototype * GenProto(Block * chunk);


#endif //CH17_CODEGEN_H
