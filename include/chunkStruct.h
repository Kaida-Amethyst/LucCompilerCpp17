//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_CHUNKSTRUCT_H
#define LUACOMPILER_RENEW_CHUNKSTRUCT_H

#define LUACOMPILER_BIN_CHUNCK_H
#define LUA_SIGNATURE "\xbLua"
#define LUAC_VERSION 0x53
#define LUAC_FORMAT 0x00
#define LUAC_DATA "\x19\x93\r\n\x1a\n"
#define CINT_SIZE 0x04
#define CSZIET_SIZE 0x08
#define INSTRUCTION_SIZE 0x04
#define LUA_INTEGER_SIZE 0x08
#define LUA_NUMBER_SIZE 0x08
#define LUAC_INT 0x5678
#define LUAC_NUM 370.5

#include <cstdint>
#include <string>
#include <vector>
#include <array>

class luaValue;

using byte = unsigned char;

extern int TAG_NIL;
extern int TAG_BOOLEN;
extern int TAG_NUMBER;
extern int TAG_INTEGER;
extern int TAG_SHORT_STR;
extern int TAG_LONG_STR;

struct __header;
struct Prototype;
struct binaryChunk;

struct Upvalue{
    byte Instack;
    byte Idx;
};

struct LocVar{
    std::string VarName;
    uint32_t    StartPC;
    uint32_t    EndPC;
};


struct __header{
    std::array<byte, 4> signature;
    byte                version;
    byte                formate;
    std::array<byte, 6> luacData[6];
    byte                cintSize;
    byte                sizetSize;
    byte                instructionSize;
    byte                luaIntegerSize;
    byte                luaNumberSizel;
    int64_t             luacInt;
    double              luacNum;
};

struct Prototype{
    std::string             Source;
    uint32_t                LineDefined{};
    uint32_t                LastLineDefined{};
    byte                    NumParams{};
    byte                    IsVararg{};
    byte                    MaxStackSize{};
    std::vector<uint32_t>  *Code;
    std::vector<luaValue>  *Constants;
    std::vector<Upvalue>   *Upvalues;
    std::vector<Prototype*>* Protos;
    std::vector<uint32_t>  *LineInfo;
    std::vector<LocVar>    *LocVars;
    std::vector<std::string>* UpvalueNames;
    Prototype() = default;
};

struct binaryChunk{
    __header    header;
    byte        sizeUpValues;
    Prototype * mainFunc;
};

Prototype* Undump(byte* data);



#endif //LUACOMPILER_RENEW_CHUNKSTRUCT_H
