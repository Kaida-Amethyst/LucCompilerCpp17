//
// Created by ziyue on 2021/6/7.
//

#ifndef CH16_OPTIMIZER_H
#define CH16_OPTIMIZER_H

#include "block.h"

bool isFalse(Exp * exp);
bool isTrue(Exp * exp);
std::pair<int64_t, bool> castToInt(Exp * exp);
std::pair<double, bool> castToFloat(Exp * exp);
bool isVarargOrFuncCall(Exp * exp);
std::vector<Exp*> * removeTailNils(std::vector<Exp*> * exps);

namespace Optimizer{
    Exp * logicalOr(BinopExp * exp);
    Exp * logicalAnd(BinopExp * exp);
    Exp * bitWiseBinaryOp(BinopExp * exp);
    Exp * arithBinaryOp(BinopExp * exp);
    Exp * pow(Exp * exp);
    Exp * unaryOp(UnopExp * exp);
    Exp * unm(UnopExp * exp);
    Exp * Not(UnopExp * exp);
    Exp * Bnot(UnopExp * exp);
}

#endif //CH16_OPTIMIZER_H
