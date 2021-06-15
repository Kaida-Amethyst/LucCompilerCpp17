//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_ARITH_H
#define LUACOMPILER_RENEW_ARITH_H

#include <cstdint>
#include "luaMath.h"
#include <cmath>
#include <functional>
#include <vector>
#include <iostream>

namespace arith{
    int64_t iadd (int64_t a, int64_t b);
    double  fadd (double  a, double  b);
    int64_t isub (int64_t a, int64_t b);
    double  fsub (double  a, double  b);
    int64_t imul (int64_t a, int64_t b);
    double  fmul (double  a, double  b);
    int64_t imod (int64_t a, int64_t b);
    double  fmod (double  a, double  b);
    double  pow  (double  a, double  b);
    double  div  (double  a, double  b);
    int64_t iidiv(int64_t a, int64_t b);
    double  fidiv(double  a, double  b);
    int64_t band (int64_t a, int64_t b);
    int64_t bor  (int64_t a, int64_t b);
    int64_t bxor (int64_t a, int64_t b);
    int64_t shl  (int64_t a, int64_t b);
    int64_t shr  (int64_t a, int64_t b);
    int64_t iunm (int64_t a, int64_t b);
    double  funm (double  a, double  b);
    int64_t bnot (int64_t a, int64_t b);
};

struct Operator{
    std::function<int64_t (int64_t, int64_t)> IntegerFunc;
    std::function<double  (double,   double)> FloatFunc;
};

extern std::vector<Operator> Operators;

#endif //LUACOMPILER_RENEW_ARITH_H
