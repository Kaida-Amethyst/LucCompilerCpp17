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
        Operator{"__add", arith::iadd, arith::fadd},
        Operator{"__sub", arith::isub, arith::fsub},
        Operator{"__mul", arith::imul, arith::fmul},
        Operator{"__mod", arith::imod, arith::fmod},
        Operator{"__pow", nullptr, arith::pow},
        Operator{"__div", nullptr, arith::div},
        Operator{"__idiv", arith::iidiv, arith::fidiv},
        Operator{"__band", arith::band, nullptr},
        Operator{"__bor", arith::bor, nullptr},
        Operator{"__bxor", arith::bxor, nullptr},
        Operator{"__shl", arith::shl, nullptr},
        Operator{"__shr", arith::shr, nullptr},
        Operator{"__unm", arith::iunm, nullptr},
        Operator{"__bnot", arith::bnot, nullptr},
};