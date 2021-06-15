//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_OPCODES_H
#define LUACOMPILER_RENEW_OPCODES_H

#include "chunkStruct.h"
#include "instruction.h"
#include <string>
#include <vector>
#include <functional>


/* instruction mode */
enum {
    IABC,
    IABx,
    IAsBx,
    IAx
};

/* instruction code */

enum {
    OP_MOVE,
    OP_LOADK,
    OP_LOADKX,
    OP_LOADBOOL,
    OP_LOADNIL,
    OP_GETUPVAL,
    OP_GETTABUP,
    OP_GETTABLE,
    OP_SETTABUP,
    OP_SETUPVAL,
    OP_SETTABLE,
    OP_NEWTABLE,
    OP_SELF,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_MOD,
    OP_POW,
    OP_DIV,
    OP_IDIV,
    OP_BAND,
    OP_BOR,
    OP_BXOR,
    OP_SHL,
    OP_SHR,
    OP_UNM,
    OP_BNOT,
    OP_NOT,
    OP_LEN,
    OP_CONCAT,
    OP_JMP,
    OP_EQ,
    OP_LT,
    OP_LE,
    OP_TEST,
    OP_TESTSET,
    OP_CALL,
    OP_TAILCALL,
    OP_RETURN,
    OP_FORLOOP,
    OP_FORPREP,
    OP_TFORCALL,
    OP_TFORLOOP,
    OP_SETLIST,
    OP_CLOSURE,
    OP_VARARG,
    OP_EXTRAARG
};

/* operator number types
 *
 * OpArgN : will not be used
 * OpArgU : bool, int, and any other type
 * OpArgR : Register index
 * OpArgK : Register or constants table index
 *
 * */

enum{
    OpArgN,
    OpArgU,
    OpArgR,
    OpArgK
};

struct opcode{
    byte testFlag;
    byte setAFlag;
    byte argBMode;
    byte argCMode;
    byte opMode;
    std::string name;
    std::function<void(Instruction &, LuaVM &)> action;
};

extern std::vector<opcode> opcodes;

#endif //LUACOMPILER_RENEW_OPCODES_H
