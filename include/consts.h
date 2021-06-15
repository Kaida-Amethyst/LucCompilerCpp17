//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_CONSTS_H
#define LUACOMPILER_RENEW_CONSTS_H

#include <cstdint>

extern const int LUA_MINSTACK;
extern const int LUAI_MAXSTACK;
extern const int LUA_REGISTRYINDEX;
extern const int64_t LUA_RIDX_GLOBALS;

enum LuaType {
    LUA_TNONE = -1,
    LUA_TNIL,
    LUA_TBOOLEAN,
    LUA_TLIGHTUSERDATA,
    LUA_TNUMBER,
    LUA_TSTRING,
    LUA_TTABLE,
    LUA_TFUNCTION,
    LUA_TUSERDATA,
    LUA_TTHREAD
};

enum ArithOp{
    LUA_OPADD ,       // +
    LUA_OPSUB ,       // -
    LUA_OPMUL ,       // *
    LUA_OPMOD ,       // %
    LUA_OPPOW ,       // ^
    LUA_OPDIV ,       // /
    LUA_OPIDIV,       // //
    LUA_OPBAND,       // &
    LUA_OPBOR ,       // |
    LUA_OPBXOR,       // ~
    LUA_OPSHL ,       // <<
    LUA_OPSHR ,       // >>
    LUA_OPUNM ,       // -
    LUA_OPBNOT        // ~
};

enum CompareOp{
    LUA_OPEQ ,      // ==
    LUA_OPLT ,      // <
    LUA_OPLE        // <=
};

#endif //LUACOMPILER_RENEW_CONSTS_H
