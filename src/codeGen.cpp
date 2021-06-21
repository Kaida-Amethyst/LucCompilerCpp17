//
// Created by ziyue on 2021/6/18.
//

#include "codeGen.h"
#include <iostream>
#include "optimizer.h"
#include "opcodes.h"
#include "token.h"
#include "luaValue.h"
#include "funcInfo.h"

void cgBlock(funcInfo * fi, Block * node){
    for(auto & stat : *node->Stats){
        cgStat(fi, stat);
    }
    if (node->RetExps != nullptr){
        cgRetStat(fi, node->RetExps);
    }
}

void cgRetStat(funcInfo * fi, std::vector<Exp*> * exps){
    auto nExps = static_cast<int>(exps->size());
    if (nExps == 0){
        fi->emitReturn(0, 0);
        return ;
    }
    if (nExps == 1){
        if (exps->at(0)->type() == EXP_NAME){
            auto nameExp = dynamic_cast<NameExp*>(exps->at(0));
            if (auto r = fi->slotOfLocVar(nameExp->Name); r >= 0){
                fi->emitReturn(r, 1);
            }
        }
        if (exps->at(0)->type() == EXP_FUNC_CALL){
            auto fcExp = dynamic_cast<FuncCallExp*>(exps->at(0));
            auto r = fi->allocReg();
            cgTailCallExp(fi, fcExp, r);
            fi->freeReg();
            fi->emitReturn(r, -1);
            return;
        }
    }

    try{
        auto multRet = isVarargOrFuncCall(exps->at(nExps - 1));
        for(auto i = 0; i < exps->size(); i++){
            auto & exp = exps->at(i);
            auto r = fi->allocReg();
            if (i == nExps -1 && multRet){
                cgExp(fi, exp, r, -1);
            }else{
                cgExp(fi, exp, r, 1);
            }
        }
        fi->freeRegs(nExps);
        auto a = fi->usedRegs;
        if (multRet){
            fi->emitReturn(a, -1);
        }else{
            fi->emitReturn(a, nExps);
        }
    }catch (...){
        std::cerr << "Error ! Location: cgRetStat, exps->size() = " << exps->size()
        << " nExps - 1 = " << nExps - 1 << std::endl;
        abort();
    }
}

void cgStat(funcInfo * fi, Stat * node){
    switch (node->type()) {
        case STAT_FUNC_CALL:
            cgFuncCallStat(fi, dynamic_cast<FuncCallStat*>(node));
            break;
        case STAT_BREAK:
            cgBreakStat(fi, dynamic_cast<BreakStat*>(node));
            break;
        case STAT_DO:
            cgDoStat(fi, dynamic_cast<DoStat*>(node));
            break;
        case STAT_REPEAT:
            cgRepeatStat(fi, dynamic_cast<RepeatStat*>(node));
            break;
        case STAT_WHILE:
            cgWhileStat(fi, dynamic_cast<WhileStat*>(node));
            break;
        case STAT_IF:
            cgIfStat(fi, dynamic_cast<IfStat*>(node));
            break;
        case STAT_FOR_NUM:
            cgForNumStat(fi, dynamic_cast<ForNumStat*>(node));
            break;
        case STAT_FOR_IN:
            cgForInStat(fi, dynamic_cast<ForInStat*>(node));
            break;
        case STAT_ASSIGN:
            cgAssignStat(fi, dynamic_cast<AssignStat*>(node));
            break;
        case STAT_LOCAL_VAR_DECL:
            cgLocalVarDeclStat(fi, dynamic_cast<LocalVarDeclStat*>(node));
            break;
        case STAT_LOCAL_FUNC_DEF:
            cgLocalFuncDefStat(fi, dynamic_cast<LocalFuncDefStat*>(node));
            break;
        default:
            std::cerr << "Not support! Location: cgStat, node->type() = " << node->type() << std::endl;
            abort();
    }
}

void cgFuncCallStat(funcInfo * fi, FuncCallStat * stat){
    auto r = fi->allocReg();
    cgFuncCallExp(fi, stat->fcexp, r, 0);
    fi->freeReg();
}

void cgBreakStat(funcInfo * fi, BreakStat * stat){
    auto pc = fi->emitJmp(0, 0);
    fi->addBreakJmp(pc);
}

void cgDoStat(funcInfo * fi, DoStat * stat){
    fi->enterScope(false);
    cgBlock(fi, stat->block);
    fi->closeOpenUpvals();
    fi->exitScope();
}

void cgWhileStat(funcInfo * fi, WhileStat * stat){
    auto pcBeforeExp = fi->pc();
    auto r = fi->allocReg();
    cgExp(fi, stat->exp, r, 1);
    fi->freeReg();
    fi->emitTest(r, 0);
    auto pcJmpToEnd = fi->emitJmp(0, 0);
    fi->enterScope(true);
    cgBlock(fi, stat->block);
    fi->closeOpenUpvals();
    fi->emitJmp(0, pcBeforeExp - fi->pc() - 1);
    fi->exitScope();
    fi->fixSbx(pcJmpToEnd, fi->pc() - pcJmpToEnd);
}

void cgRepeatStat(funcInfo * fi, RepeatStat * stat){
    fi->enterScope(true);
    auto pcBeforeBlock = fi->pc();
    cgBlock(fi, stat->block);
    auto r = fi->allocReg();
    cgExp(fi, stat->exp, r, 1);
    fi->freeReg();
    fi->emitTest(r, 0);
    fi->emitJmp(fi->getJmpArgA(), pcBeforeBlock - fi->pc() - 1);
    fi->closeOpenUpvals();
    fi->exitScope();
}

void cgIfStat(funcInfo * fi, IfStat * stat){
    auto pcJmpToEnds = new std::vector<int>(stat->Exps->size());
    auto pcJmpToNextExp = -1;
    for(auto i = 0; i < stat->Exps->size(); i++){
        auto & exp = stat->Exps->at(i);
        if (pcJmpToNextExp >= 0){
            fi->fixSbx(pcJmpToNextExp, fi->pc() - pcJmpToNextExp);
        }
        auto r = fi->allocReg();
        cgExp(fi, exp, r, 1);
        fi->freeReg();
        fi->emitTest(r, 0);
        pcJmpToNextExp = fi->emitJmp(0, 0);
        fi->enterScope(false);
        cgBlock(fi, stat->blocks->at(i));
        fi->closeOpenUpvals();
        fi->exitScope();
        if (i < stat->Exps->size() - 1){
            pcJmpToEnds->at(i) = fi->emitJmp(0, 0);
        }else{
            pcJmpToEnds->at(i) = pcJmpToNextExp;
        }
    }
    for(auto & pc : *pcJmpToEnds){
        fi->fixSbx(pc, fi->pc() - pc);
    }
}

void cgForNumStat(funcInfo * fi, ForNumStat * stat){
    fi->enterScope(true);
    auto lvds = new LocalVarDeclStat(
            0, new std::vector<std::string>{"(for index)", "(for limit)", "(for step)"},
            new std::vector<Exp*>{stat->InitExp, stat->LimitExp, stat->StepExp});
    cgLocalVarDeclStat(fi, lvds);
    fi->addLocVar(stat->VarName);
    auto a = fi->usedRegs - 4;
    auto pcForPrep = fi->emitForPrep(a, 0);
    cgBlock(fi, stat->block);
    fi->closeOpenUpvals();
    auto pcForLoop = fi->emitForLoop(a, 0);
    fi->fixSbx(pcForPrep, pcForLoop - pcForPrep - 1);
    fi->fixSbx(pcForLoop, pcForPrep - pcForLoop);
    fi->exitScope();
}

void cgForInStat(funcInfo * fi, ForInStat * stat){
    fi->enterScope(true);
    auto lvds = new LocalVarDeclStat(
            0, new std::vector<std::string>{"(for generator)", "(for state)", "(for control)"},
            stat->ExpList);
    cgLocalVarDeclStat(fi, lvds);
    for(auto & name : *stat->NameList){
        fi->addLocVar(name);
    }
    auto pcJmpToTFC = fi->emitJmp(0, 0);
    cgBlock(fi, stat->block);
    fi->closeOpenUpvals();
    fi->fixSbx(pcJmpToTFC, fi->pc() - pcJmpToTFC);
    auto rGenerator = fi->slotOfLocVar("(for generator)");
    fi->emitTForCall(rGenerator, static_cast<int>(stat->NameList->size()));
    fi->emitTForLoop(rGenerator + 2, pcJmpToTFC - fi->pc() - 1);
    fi->exitScope();
}

void cgLocalVarDeclStat(funcInfo * fi, LocalVarDeclStat * stat){
    // exps := removeTailNils(node.ExpList)
    auto exps = removeTailNils(stat->ExpList);
    auto nExps = static_cast<int>(exps->size());
    auto nNames = static_cast<int>(stat->NameList->size());
    auto oldRegs = fi->usedRegs;
    if (nExps == nNames){
        for(auto & exp : *exps){
            auto a = fi->allocReg();
            cgExp(fi, exp, a, 1);
        }
    }else if (nExps > nNames){
        for(auto i = 0; i < exps->size(); i++){
            auto & exp = exps->at(i);
            auto a = fi->allocReg();
            if (i == nExps - 1 && isVarargOrFuncCall(exp)){
                cgExp(fi, exp, a, 0);
            }else{
                cgExp(fi, exp, a, 1);
            }
        }
    }else{
        auto multRet = false;
        for(auto i = 0; i < exps->size(); i++){
            auto & exp = exps->at(i);
            auto a = fi->allocReg();
            if (i == nExps - 1 && isVarargOrFuncCall(exp)){
                multRet = true;
                auto n = nNames - nExps + 1;
                cgExp(fi, exp, a, n);
                fi->allocRegs(n - 1);
            }else{
                cgExp(fi, exp, a, 1);
            }
        }
        if (!multRet){
            auto n = nNames - nExps;
            auto a = fi->allocRegs(n);
            fi->emitLoadNil(a, n);
        }
    }
    fi->usedRegs = oldRegs;
    for(auto & name : *stat->NameList){
        fi->addLocVar(name);
    }
}

void cgAssignStat(funcInfo * fi, AssignStat * stat) {
    auto exps = removeTailNils(stat->ExpList);
    auto nExps = static_cast<int>(exps->size());
    auto nVars = static_cast<int>(stat->VarList->size());
    auto oldRegs = fi->usedRegs;
    auto tRegs = new std::vector<int>(nVars);
    auto kRegs = new std::vector<int>(nVars);
    auto vRegs = new std::vector<int>(nVars);

    for(auto i = 0; i < stat->VarList->size(); i++){
        auto & exp = stat->VarList->at(i);
        if (exp->type() == EXP_TABLE_ACCESS){
            auto taExp = dynamic_cast<TableAccessExp*>(exp);
            tRegs->at(i) = fi->allocReg();
            cgExp(fi, taExp->PrefixExp, tRegs->at(i), 1);
            kRegs->at(i) = fi->allocReg();
            cgExp(fi, taExp->KeyExp, kRegs->at(i), 1);
        }
    }
    for(auto i = 0; i < nVars; i++){
        vRegs->at(i) = fi->usedRegs + i;
    }
    if (nExps >= nVars){
        for(auto i = 0; i< exps->size(); i++){
            auto & exp = exps->at(i);
            auto a = fi->allocReg();
            if (i >= nVars && i == nExps - 1 && isVarargOrFuncCall(exp)){
                cgExp(fi, exp, a, 0);
            }else{
                cgExp(fi, exp, a, 1);
            }
        }
    }else{
        auto multRet = false;
        for(auto i = 0; i < exps->size(); i++){
            auto & exp = exps->at(i);
            auto a = fi->allocReg();
            if (i == nExps - 1 && isVarargOrFuncCall(exp)){
                multRet = true;
                auto n = nVars - nExps + 1;
                cgExp(fi, exp, a, n);
                fi->allocRegs(n-1);
            }else{
                cgExp(fi, exp, a, 1);
            }
        }
        if (!multRet){
            auto n = nVars - nExps;
            auto a = fi->allocRegs(n);
            fi->emitLoadNil(a, n);
        }
    }
    for(auto i = 0; i<stat->VarList->size();i++){
        auto & exp = stat->VarList->at(i);
        if (exp->type() == EXP_NAME){
            auto nameExp = dynamic_cast<NameExp*>(exp);
            auto varName = nameExp->Name;
            if (auto a = fi->slotOfLocVar(varName); a >= 0){
                fi->emitMove(a, vRegs->at(i));
            }else if (auto b = fi->indexOfUpval(varName); b>=0){
                fi->emitSetUpval(vRegs->at(i), b);
            }else{
                auto x = fi->indexOfUpval("_ENV");
                auto y = 0x100 + fi->indexOfConstant(luaValue(varName));
                fi->emitSetTabUp(x, y, vRegs->at(i));
            }
        }else{
            fi->emitSetTable(tRegs->at(i), kRegs->at(i), vRegs->at(i));
        }
    }
    fi->usedRegs = oldRegs;
}

void cgLocalFuncDefStat(funcInfo * fi, LocalFuncDefStat * stat){
    auto r = fi->addLocVar(stat->Name);
    cgFuncDefExp(fi, stat->fde, r);
}


void cgExp(funcInfo * fi, Exp * node, int a, int n){
    switch (node->type()) {
        case EXP_NIL:
            fi->emitLoadNil(a, n);
            break;
        case EXP_FALSE:
            fi->emitLoadBool(a, 0, 0);
            break;
        case EXP_TRUE:
            fi->emitLoadBool(a, 1, 0);
            break;
        case EXP_INTEGER:
            fi->emitLoadK(a, luaValue(dynamic_cast<IntegerExp*>(node)->Val));
            break;
        case EXP_FLOAT:
            fi->emitLoadK(a, luaValue(dynamic_cast<FloatExp*>(node)->Val));
            break;
        case EXP_STRING:
            fi->emitLoadK(a, luaValue(dynamic_cast<StringExp*>(node)->Str));
            break;
        case EXP_PARENS:
            cgExp(fi, dynamic_cast<ParensExp*>(node)->exp, a, 1);
            break;
        case EXP_VARARG:
            cgVarargExp(fi, dynamic_cast<VarargExp*>(node), a, n);
            break;
        case EXP_FUNC_DEF:
            cgFuncDefExp(fi, dynamic_cast<FuncDefExp*>(node), a);
            break;
        case EXP_TABLE_CONSTRUCTOR:
            cgTableConstructorExp(fi, dynamic_cast<TableConstructorExp*>(node), a);
            break;
        case EXP_UNOP:
            cgUnopExp(fi, dynamic_cast<UnopExp*>(node), a);
            break;
        case EXP_BINOP:
            cgBinopExp(fi, dynamic_cast<BinopExp*>(node), a);
            break;
        case EXP_CONCAT:
            cgConcatExp(fi, dynamic_cast<ConcatExp*>(node), a);
            break;
        case EXP_NAME:
            cgNameExp(fi, dynamic_cast<NameExp*>(node), a);
            break;
        case EXP_TABLE_ACCESS:
            cgTableAccessExp(fi, dynamic_cast<TableAccessExp*>(node), a);
            break;
        case EXP_FUNC_CALL:
            cgFuncCallExp(fi, dynamic_cast<FuncCallExp*>(node), a, n);
            break;
    }
}

void cgVarargExp(funcInfo * fi, VarargExp * exp, int a, int n){
    if (!fi->isVararg ){
        std::cerr << "cannot use '...' outside a cararg function!" << std::endl;
        abort();
    }
    fi->emitVararg(a, n);
}

void cgFuncDefExp(funcInfo * fi, FuncDefExp * exp, int a){
    auto subFI = new funcInfo(fi, exp);
    fi->subFuncs->push_back(subFI);
    if (exp->ParList != nullptr){
        for(auto & param: *exp->ParList){
            subFI->addLocVar(param);
        }
    }
    cgBlock(subFI, exp->block);
    subFI->exitScope();
    subFI->emitReturn(0, 0);
    auto bx = static_cast<int>(fi->subFuncs->size()) - 1;
    fi->emitClosure(a, bx);
}

void cgTableConstructorExp(funcInfo * fi, TableConstructorExp * exp, int a){
    auto nArr = 0;
    for(auto & keyExp : *exp->KeyExps){
        if (keyExp == nullptr) nArr ++;
    }
    auto nExps = static_cast<int>(exp->KeyExps->size());
    auto mulRet = nExps > 0 && isVarargOrFuncCall(exp->ValExps->at(nExps - 1));
    fi->emitNewTable(a, nArr, nExps - nArr);
    auto arrIdx = 0;
    for(auto i=0;i<exp->KeyExps->size(); i++){
        auto & keyExp = exp->KeyExps->at(i);
        auto & valExp = exp->ValExps->at(i);
        if (keyExp == nullptr){
            arrIdx ++;
            auto tmp = fi->allocReg();
            if (i == nExps - 1 && mulRet){
                cgExp(fi, valExp, tmp, -1);
            }else{
                cgExp(fi, valExp, tmp, 1);
            }
            if (arrIdx % 50 == 0 || arrIdx == nArr){
                auto n = arrIdx % 50;
                if (n == 0){n = 50;};
                auto c = (arrIdx - 1) / 50 + 1;
                fi->freeRegs(n);
                if (i == nExps - 1 && mulRet){
                    fi->emitSetList(a, 0, c);
                } else {
                    fi->emitSetList(a, n, c);
                }
            }
            continue;
        }

        auto b = fi->allocReg();
        cgExp(fi, keyExp, b, 1);
        auto c = fi->allocReg();
        cgExp(fi, valExp, c, 1);
        fi->freeRegs(2);
        fi->emitSetTable(a, b, c);
    }
}

void cgUnopExp(funcInfo * fi, UnopExp * exp, int a){
    auto b = fi->allocReg();
    cgExp(fi, exp->exp, b, 1);
    fi->emitUnaryOp(exp->Op, a, b);
    fi->freeReg();
}

void cgConcatExp(funcInfo * fi, ConcatExp * exp, int a){
    for(auto & subExp : *exp->exps){
        auto x = fi->allocReg();
        cgExp(fi, subExp, x, 1);
    }
    auto c = fi->usedRegs - 1;
    auto b = c - static_cast<int>(exp->exps->size()) + 1;
    fi->freeRegs(c - b + 1);
    fi->emitABC(OP_CONCAT, a, b, c);
}

void cgBinopExp(funcInfo * fi, BinopExp * exp, int a){
    switch (exp->Op) {
        case TOKEN_OP_AND:
        case TOKEN_OP_OR:
        {
            auto b = fi->allocReg();
            cgExp(fi, exp->Exp1, b, 1);
            fi->freeReg();
            if (exp->Op == TOKEN_OP_AND){
                fi->emitTestSet(a, b, 0);
            } else {
                fi->emitTestSet(a, b, 1);
            }
            auto pcOfJmp = fi->emitJmp(0, 0);
            b = fi->allocReg();
            cgExp(fi, exp->Exp2, b, 1);
            fi->freeReg();
            fi->emitMove(a, b);
            fi->fixSbx(pcOfJmp, fi->pc() - pcOfJmp);
        }
        default:
        {
            auto b = fi->allocReg();
            cgExp(fi, exp->Exp1, b, 1);
            auto c = fi->allocReg();
            cgExp(fi, exp->Exp2, c, 1);
            fi->emitBinaryOp(exp->Op, a, b, c);
            fi->freeRegs(2);
        }
    }
}



void cgNameExp(funcInfo * fi, NameExp * exp, int a){
    if (auto r = fi->slotOfLocVar(exp->Name); r >= 0){
        fi->emitMove(a, r);
    } else if (auto idx = fi->indexOfUpval(exp->Name); idx >= 0){
        fi->emitGetUpval(a, idx);
    } else {
        auto taExp = new TableAccessExp(0, new NameExp{0, "_ENV"},
                                        new StringExp{0, exp->Name});
        cgTableAccessExp(fi, taExp, a);
    }
}

void cgTableAccessExp(funcInfo * fi, TableAccessExp * exp, int a){
    auto b = fi->allocReg();
    cgExp(fi, exp->PrefixExp, b, 1);
    auto c = fi->allocReg();
    cgExp(fi, exp->KeyExp, c, 1);
    fi->emitGetTable(a, b, c);
    fi->freeRegs(2);
}

static int prepFuncCall(funcInfo * fi, FuncCallExp * node, int a){
    auto nArgs = static_cast<int>(node->Args->size());
    auto lastArgsIsVarargOrFuncCall = false;
    cgExp(fi, node->PrefixExp, a, 1);
    if (node->NameExp != nullptr){
        auto c = 0x100 + fi->indexOfConstant(luaValue(node->NameExp->Str));
        fi->emitSelf(a, a, c);
    }
    for(auto i = 0; i < node->Args->size();i++){
        auto & arg = node->Args->at(i);
        auto tmp = fi->allocReg();
        if (i == nArgs - 1 && isVarargOrFuncCall(arg)){
            lastArgsIsVarargOrFuncCall = true;
            cgExp(fi, arg, tmp, -1);
        } else {
            cgExp(fi, arg, tmp, 1);
        }
        fi->freeRegs(nArgs);
        if (node->NameExp != nullptr){ nArgs ++ ;}
        if (lastArgsIsVarargOrFuncCall) {nArgs = -1;}
        return nArgs;
    }
}

void cgFuncCallExp(funcInfo * fi, FuncCallExp * exp, int a, int n){
    auto nArgs = prepFuncCall(fi, exp, a);
    fi->emitCall(a, nArgs, n);
}

void cgTailCallExp(funcInfo * fi, FuncCallExp * exp, int a){
    auto nArgs = prepFuncCall(fi, exp, a);
    fi->emitTailCall(a, nArgs);
}

Prototype * GenProto(Block * chunk){
    auto fd = new FuncDefExp{0, 0, nullptr, true, chunk};
    auto fi = new funcInfo(nullptr, fd);
    fi->addLocVar("_ENV");
    cgFuncDefExp(fi, fd, 0);
    return toProto(fi->subFuncs->at(0));
}
