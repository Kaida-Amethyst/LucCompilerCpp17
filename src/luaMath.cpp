//
// Created by ziyue on 2021/6/13.
//

#include "luaMath.h"


int64_t IFloorDiv(int64_t a, int64_t b){
    if (a > 0 && b > 0 || a<0 && b < 0 || a%b == 0){
        return a/b;
    }else{
        return a/b - 1;
    }
}

double FFloorDiv(double a, double b){
    return floor(a/b);
}

int64_t IMod(int64_t a, int64_t b){
//    std::cout << "a = " << a << " b= " << b << " mod = " << (a - IFloorDiv(a, b)*b) << std::endl;
    return a - IFloorDiv(a, b)*b;
}

double FMod(double a, double b){
    return a - floor(a/b)*b;
}

int64_t ShiftLeft(int64_t a, int64_t n){
    if (n >= 0){
        return a << static_cast<uint64_t>(n);
    }else{
        return ShiftRight(a, -n);
    }
}

int64_t ShiftRight(int64_t a, int64_t n){
    if (n >= 0){
        return static_cast<int64_t>(static_cast<uint64_t>(a) >> static_cast<uint64_t>(n));
    }else{
        return ShiftLeft(a, -n);
    }
}

std::pair<int64_t, bool> FloatToInteger(double f){
    auto I = static_cast<int64_t>(f);
    return std::pair{I, (static_cast<double>(I) == f)};
}