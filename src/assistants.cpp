//
// Created by ziyue on 2021/6/13.
//

#include "assistants.h"
#include "luaMath.h"
#include <regex>

namespace asf{
    bool isInteger(const std::string &str){
        if (str.empty()) return false;
        if (!std::isdigit(str.at(0)) && str.at(0) != '+' && str.at(0) != '-') return false;
        size_t idx = 0;
        std::stoi(str, &idx);
        return idx == str.size();
    }

    bool convertToBoolen(const luaValue &val){
        switch (val.type()) {
            case luaValue::Nil:
                return false;
            case luaValue::Boolean:
                return val.get<luaValue::Boolean>();
            default:
                return true;
        }
    }

    bool isNumber(const std::string &str){
        if (str.empty()) return false;
        if (!std::isdigit(str.at(0)) && str.at(0) != '+' && str.at(0) != '-') return false;
        size_t idx = 0;
        std::stod(str, &idx);
        return idx == str.size();
    }

    std::pair<double, bool> convertToFloat(const luaValue &val){
        switch (val.type()) {
            case luaValue::Int:
                return std::pair{static_cast<double>(val.get<luaValue::Int>()), true};
            case luaValue::Float:
                return std::pair{val.get<luaValue::Float>(), true};
            case luaValue::String:{
                std::string x {val.get<luaValue::String>()};
                auto isn {isNumber(x)};
                return std::pair{std::stod(x), isn};
            }
            default:
                return std::pair{0, false};
        }
    }

    std::pair<int64_t, bool> convertToInteger(const luaValue &val){
        switch (val.type()) {
            case luaValue::Int:
                return std::pair{val.get<luaValue::Int>(), true};
            case luaValue::Float:
                return std::pair{static_cast<int64_t>(val.get<luaValue::Float>()), true};
            case luaValue::String:{
                std::string x {val.get<luaValue::String>()};
                if (isInteger(x)) {
                    return std::pair{std::stoi(x), true};
                }else if (isNumber(x)){
                    auto N = std::stod(x);
                    return FloatToInteger(N);
                }else{
                    return std::pair{0, false};
                }
            }
            default:
                return std::pair{0, false};
        }
    }

    int Int2fb(int x){
        auto e = 0;
        if (x < 8) return x;
        while(x >= (8 << 4)){
            x = (x+0xf) >> 4;
            e+=4;
        }
        while(x >= (8 << 1)){
            x = (x+1) >> 1;
            e++;
        }
        return ((e+1) << 3) | (x-8);
    }

    int Fb2int(int x){
        if (x < 8) return x;
        else{
            return ((x & 7) + 8) << (static_cast<uint32_t>(x) >> 3 - 1);
        }
    }
}