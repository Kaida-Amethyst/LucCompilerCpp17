//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_LUAMATH_H
#define LUACOMPILER_RENEW_LUAMATH_H

#include <cstdint>
#include <cmath>

int64_t IFloorDiv(int64_t a, int64_t b);

double FFloorDiv(double a, double b);

int64_t IMod(int64_t a, int64_t b);

double FMod(double a, double b);

int64_t ShiftLeft(int64_t a, int64_t n);

int64_t ShiftRight(int64_t a, int64_t n);

std::pair<int64_t, bool> FloatToInteger(double f);

#endif //LUACOMPILER_RENEW_LUAMATH_H
