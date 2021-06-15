//
// Created by ziyue on 2021/6/13.
//

#include "arith.h"


namespace arith {
    int64_t iadd(int64_t a, int64_t b) {
        return a + b;
    }

    double fadd(double a, double b) {
        return a + b;
    }

    int64_t isub(int64_t a, int64_t b) {
        return a - b;
    }

    double fsub(double a, double b) {
        return a - b;
    }

    int64_t imul(int64_t a, int64_t b) {
        return a * b;
    }

    double fmul(double a, double b) {
        return a * b;
    }

    int64_t imod(int64_t a, int64_t b) {
        return IMod(a, b);
    }

    double fmod(double a, double b) {
        return FMod(a, b);
    }

    double pow(double a, double b) {
        return std::pow(a, b);
    }

    double div(double a, double b) {
        return a / b;
    }

    int64_t iidiv(int64_t a, int64_t b) {
        return IFloorDiv(a, b);
    }

    double fidiv(double a, double b) {
        return FFloorDiv(a, b);
    }

    int64_t band(int64_t a, int64_t b) {
        return a & b;
    }

    int64_t bor(int64_t a, int64_t b) {
        return a | b;
    }

    int64_t bxor(int64_t a, int64_t b) {
        return a ^ b;
    }

    int64_t shl(int64_t a, int64_t b) {
        return ShiftLeft(a, b);
    }

    int64_t shr(int64_t a, int64_t b) {
        return ShiftRight(a, b);
    }

    int64_t iunm(int64_t a, int64_t b) {
        return -a;
    }

    double funm(double a, double b) {
        return -a;
    }

    int64_t bnot(int64_t a, int64_t b) {
        return ~a;
    }
}

std::vector<Operator> Operators{
        Operator{arith::iadd, arith::fadd},
        Operator{arith::isub, arith::fsub},
        Operator{arith::imul, arith::fmul},
        Operator{arith::imod, arith::fmod},
        Operator{nullptr, arith::pow},
        Operator{nullptr, arith::div},
        Operator{arith::iidiv, arith::fidiv},
        Operator{arith::band, nullptr},
        Operator{arith::bor, nullptr},
        Operator{arith::bxor, nullptr},
        Operator{arith::shl, nullptr},
        Operator{arith::shr, nullptr},
        Operator{arith::iunm, nullptr},
        Operator{arith::bnot, nullptr},
};