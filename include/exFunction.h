//
// Created by ziyue on 2021/6/14.
//

#ifndef CH09_EXFUNCTION_H
#define CH09_EXFUNCTION_H

#include <functional>
class luaState;
class LuaVM;

using ExFunction = std::function<int(LuaVM *)>;


#endif //CH09_EXFUNCTION_H
