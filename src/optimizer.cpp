//
// Created by ziyue on 2021/6/7.
//

#include "optimizer.h"
#include "token.h"
#include <cmath>


bool isFalse(Exp * exp){
    if (exp->type() == EXP_FALSE || exp->type() == EXP_NIL){
        return true;
    }
    return false;
}

bool isTrue(Exp * exp){
    switch (exp->type()) {
        case EXP_TRUE:
        case EXP_INTEGER:
        case EXP_FLOAT:
        case EXP_STRING:
            return true;
        default:
            return false;
    }
}

std::pair<int64_t, bool> castToInt(Exp * exp){
    switch (exp->type()) {
        case EXP_INTEGER:
            return std::pair{dynamic_cast<IntegerExp*>(exp)->Val, true};
        case EXP_FLOAT:
            return std::pair{static_cast<int64_t>(dynamic_cast<FloatExp*>(exp)->Val), true};
        default:
            return std::pair{0, false};
    }
}

std::pair<double, bool> castToFloat(Exp * exp){
    switch (exp->type()) {
        case EXP_INTEGER:
            return std::pair{static_cast<double>(dynamic_cast<IntegerExp*>(exp)->Val), true};
        case EXP_FLOAT:
            return std::pair{dynamic_cast<FloatExp*>(exp)->Val, true};
        default:
            return std::pair{0, false};
    }
}

bool isVarargOrFuncCall(Exp * exp){
    switch (exp->type()) {
        case EXP_VARARG:
        case EXP_FUNC_CALL:
            return true;
        default:
            return false;
    }
}

//func removeTailNils(exps []Exp) []Exp {
//	for n := len(exps) - 1; n >= 0; n-- {
//		if _, ok := exps[n].(*NilExp); !ok {
//			return exps[0 : n+1]
//		}
//	}
//	return nil
//}

std::vector<Exp*> * removeTailNils(std::vector<Exp*> * exps){
    while(exps->back()->type() == EXP_NIL && !exps->empty()){
        exps->erase(exps->end()-1);
    }
    if (exps->empty()){
        delete exps; exps = nullptr;
        return nullptr;
    }else{
        return exps;
    }

}

Exp * Optimizer::logicalOr(BinopExp *exp) {
    if (isTrue(exp->Exp1)){
        return exp->Exp1;
    }
    if (isFalse(exp->Exp1) && !isVarargOrFuncCall(exp->Exp2)){
        return exp->Exp2;
    }
    return exp;
}

Exp * Optimizer::logicalAnd(BinopExp *exp) {
    if (isFalse(exp->Exp1)){
        return exp->Exp1;
    }
    if (isTrue(exp->Exp1) && !isVarargOrFuncCall(exp->Exp2)){
        return exp->Exp2;
    }
    return exp;
}

Exp * Optimizer::bitWiseBinaryOp(BinopExp *exp) {
    if (auto [i, ok] = castToInt(exp->Exp1); ok){
        if (auto [j, ok2] = castToInt(exp->Exp2); ok2){
            switch (exp->Op) {
                case TOKEN_OP_BAND:
                    return new IntegerExp{exp->Line, i & j};
                case TOKEN_OP_BOR:
                    return new IntegerExp{exp->Line, i | j};
                case TOKEN_OP_BXOR:
                    return new IntegerExp{exp->Line, i ^ j};
                case TOKEN_OP_SHL:
                    return new IntegerExp{exp->Line, i << j};
                case TOKEN_OP_SHR:
                    return new IntegerExp{exp->Line, i >> j};
            }
        }
    }
    return exp;
}

Exp * Optimizer::arithBinaryOp(BinopExp *exp) {
    if (exp->Exp1->type() == EXP_INTEGER){
        auto x = dynamic_cast<IntegerExp*>(exp->Exp1);
        if (exp->Exp2->type() == EXP_INTEGER){
            auto y = dynamic_cast<IntegerExp*>(exp->Exp2);
            switch (exp->Op) {
                case TOKEN_OP_ADD:
                    return new IntegerExp{exp->Line, x->Val+y->Val};
                case TOKEN_OP_SUB:
                    return new IntegerExp{exp->Line, x->Val - y->Val};
                case TOKEN_OP_MUL:
                    return new IntegerExp{exp->Line, x->Val * y->Val};
                case TOKEN_OP_IDIV:
                    if (y->Val != 0){
                        return new IntegerExp{exp->Line, x->Val / y->Val};
                    }
                case TOKEN_OP_MOD:
                    if (y->Val != 0){
                        return new IntegerExp{exp->Line, x->Val % y->Val};
                    }
            }
        }
    }
    if (auto [f, ok] = castToFloat(exp->Exp1); ok){
        if (auto [g, ok] = castToFloat(exp->Exp2); ok){
            switch (exp->Op) {
                case TOKEN_OP_ADD:
                    return new FloatExp{exp->Line, f+g};
                case TOKEN_OP_SUB:
                    return new FloatExp{exp->Line, f - g};
                case TOKEN_OP_MUL:
                    return new FloatExp{exp->Line, f * g};
                case TOKEN_OP_DIV:
                    if (g != 0){
                        return new FloatExp{exp->Line, f / g};
                    }
                case TOKEN_OP_IDIV:
                    if (g != 0){
                        return new FloatExp{exp->Line, std::floor(f /g)};
                    }
                case TOKEN_OP_MOD:
                    if (g != 0){
                        return new FloatExp{exp->Line, std::fmod(f, g)};
                    }
                case TOKEN_OP_POW:
                    return new FloatExp{exp->Line, std::pow(f, g)};
            }
        }
    }
    return exp;
}

Exp * Optimizer::pow(Exp *exp) {
    if (exp->type() == EXP_BINOP){
        auto binop = dynamic_cast<BinopExp*>(exp);
        if (binop->Op == TOKEN_OP_POW){
            binop->Exp2 = Optimizer::pow(binop->Exp2);
        }
        return Optimizer::arithBinaryOp(binop);
    }
    return exp;
}

Exp * Optimizer::unaryOp(UnopExp *exp) {
    switch (exp->Op) {
        case TOKEN_OP_UNM:
            return Optimizer::unm(exp);
        case TOKEN_OP_NOT:
            return Optimizer::Not(exp);
        case TOKEN_OP_BNOT:
            return Optimizer::Bnot(exp);
        default:
            return exp;
    }
}

Exp * Optimizer::unm(UnopExp *exp) {
    switch (exp->exp->type()) {
        case EXP_INTEGER:
        {
            auto x = dynamic_cast<IntegerExp*>(exp->exp);
            x->Val = - x->Val;
            return x;
        }
        case EXP_FLOAT:
        {
            auto x = dynamic_cast<FloatExp*>(exp->exp);
            x->Val = - x->Val;
            return x;
        }
    }
    return exp;
}

Exp * Optimizer::Not(UnopExp *exp) {
    switch (exp->exp->type()) {
        case EXP_NIL:
        case EXP_FALSE:
            return new TrueExp{exp->Line};
        case EXP_TRUE:
        case EXP_INTEGER:
        case EXP_FLOAT:
        case EXP_STRING:
            return new FalseExp{exp->Line};
        default:
            return exp;
    }
}

Exp * Optimizer::Bnot(UnopExp *exp) {
    switch (exp->exp->type()) {
        case EXP_INTEGER:
        {
            auto x = dynamic_cast<IntegerExp*>(exp->exp);
            x->Val = ~ x->Val;
            return x;
        }
        case EXP_FLOAT:
        {
            auto x = dynamic_cast<FloatExp*>(exp->exp);
            auto i = static_cast<int64_t>(x->Val);
            return new IntegerExp{x->Line, ~i};
        }
        default:
            return exp;
    }

}