//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_ASSISTANTS_H
#define LUACOMPILER_RENEW_ASSISTANTS_H

#include <string>
#include "luaValue.h"

namespace asf{
    bool isInteger(const std::string &str);
    bool convertToBoolen(const luaValue &val);
    bool isNumber(const std::string &str);
    std::pair<double, bool> convertToFloat(const luaValue &val);
    std::pair<int64_t, bool> convertToInteger(const luaValue &val);
    int Int2fb(int x);
    int Fb2int(int x);
};

#endif //LUACOMPILER_RENEW_ASSISTANTS_H
