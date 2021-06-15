//
// Created by ziyue on 2021/6/13.
//

#include "opcodes.h"
#include "instruction.h"


std::vector<opcode> opcodes{
        opcode{0, 1, OpArgR, OpArgN, IABC /* */, "MOVE    ", move},     // R(A) := R(B)
        opcode{0, 1, OpArgK, OpArgN, IABx /* */, "LOADK   ", loadK},    // R(A) := Kst(Bx)
        opcode{0, 1, OpArgN, OpArgN, IABx /* */, "LOADKX  ", loadKx},   // R(A) := Kst(extra arg)
        opcode{0, 1, OpArgU, OpArgU, IABC /* */, "LOADBOOL", loadBool}, // R(A) := (bool)B; if (C) pc++
        opcode{0, 1, OpArgU, OpArgN, IABC /* */, "LOADNIL ", loadNil},  // R(A), R(A+1), ..., R(A+B) := nullptr
        opcode{0, 1, OpArgU, OpArgN, IABC /* */, "GETUPVAL", nullptr},      // R(A) := UpValue[B]
        opcode{0, 1, OpArgU, OpArgK, IABC /* */, "GETTABUP", nullptr},      // R(A) := UpValue[B][RK(C)]
        opcode{0, 1, OpArgR, OpArgK, IABC /* */, "GETTABLE", getTable},      // R(A) := R(B)[RK(C)]
        opcode{0, 0, OpArgK, OpArgK, IABC /* */, "SETTABUP", nullptr},      // UpValue[A][RK(B)] := RK(C)
        opcode{0, 0, OpArgU, OpArgN, IABC /* */, "SETUPVAL", nullptr},      // UpValue[B] := R(A)
        opcode{0, 0, OpArgK, OpArgK, IABC /* */, "SETTABLE", setTable},      // R(A)[RK(B)] := RK(C)
        opcode{0, 1, OpArgU, OpArgU, IABC /* */, "NEWTABLE", newTable},      // R(A) := {} (size = B,C)
        opcode{0, 1, OpArgR, OpArgK, IABC /* */, "SELF    ", self},      // R(A+1) := R(B); R(A) := R(B)[RK(C)]
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "ADD     ", add},      // R(A) := RK(B) + RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "SUB     ", sub},      // R(A) := RK(B) - RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "MUL     ", mul},      // R(A) := RK(B) * RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "MOD     ", mod},      // R(A) := RK(B) % RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "POW     ", Pow},      // R(A) := RK(B) ^ RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "DIV     ", Div},      // R(A) := RK(B) / RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "IDIV    ", idiv},     // R(A) := RK(B) // RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "BAND    ", band},     // R(A) := RK(B) & RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "BOR     ", bor},      // R(A) := RK(B) | RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "BXOR    ", bxor},     // R(A) := RK(B) ~ RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "SHL     ", shl},      // R(A) := RK(B) << RK(C)
        opcode{0, 1, OpArgK, OpArgK, IABC /* */, "SHR     ", shr},      // R(A) := RK(B) >> RK(C)
        opcode{0, 1, OpArgR, OpArgN, IABC /* */, "UNM     ", unm},      // R(A) := -R(B)
        opcode{0, 1, OpArgR, OpArgN, IABC /* */, "BNOT    ", bnot},     // R(A) := ~R(B)
        opcode{0, 1, OpArgR, OpArgN, IABC /* */, "NOT     ", Not},      // R(A) := not R(B)
        opcode{0, 1, OpArgR, OpArgN, IABC /* */, "LEN     ", length},   // R(A) := length of R(B)
        opcode{0, 1, OpArgR, OpArgR, IABC /* */, "CONCAT  ", concat},   // R(A) := R(B).. ... ..R(C)
        opcode{0, 0, OpArgR, OpArgN, IAsBx /**/, "JMP     ", jmp},      // pc+=sBx; if (A) close all upvalues >= R(A - 1)
        opcode{1, 0, OpArgK, OpArgK, IABC /* */, "EQ      ", eq},       // if ((RK(B) == RK(C)) ~= A) then pc++
        opcode{1, 0, OpArgK, OpArgK, IABC /* */, "LT      ", lt},       // if ((RK(B) <  RK(C)) ~= A) then pc++
        opcode{1, 0, OpArgK, OpArgK, IABC /* */, "LE      ", le},       // if ((RK(B) <= RK(C)) ~= A) then pc++
        opcode{1, 0, OpArgN, OpArgU, IABC /* */, "TEST    ", test},     // if not (R(A) <=> C) then pc++
        opcode{1, 1, OpArgR, OpArgU, IABC /* */, "TESTSET ", testSet},  // if (R(B) <=> C) then R(A) := R(B) else pc++
        opcode{0, 1, OpArgU, OpArgU, IABC /* */, "CALL    ", call},      // R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1))
        opcode{0, 1, OpArgU, OpArgU, IABC /* */, "TAILCALL", tailCall},      // return R(A)(R(A+1), ... ,R(A+B-1))
        opcode{0, 0, OpArgU, OpArgN, IABC /* */, "RETURN  ", _return},      // return R(A), ... ,R(A+B-2)
        opcode{0, 1, OpArgR, OpArgN, IAsBx /**/, "FORLOOP ", forLoop},  // R(A)+=R(A+2); if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }
        opcode{0, 1, OpArgR, OpArgN, IAsBx /**/, "FORPREP ", forPrep},  // R(A)-=R(A+2); pc+=sBx
        opcode{0, 0, OpArgN, OpArgU, IABC /* */, "TFORCALL", nullptr},      // R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));
        opcode{0, 1, OpArgR, OpArgN, IAsBx /**/, "TFORLOOP", nullptr},      // if R(A+1) ~= nullptr then { R(A)=R(A+1); pc += sBx }
        opcode{0, 0, OpArgU, OpArgU, IABC /* */, "SETLIST ", setList},      // R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B
        opcode{0, 1, OpArgU, OpArgN, IABx /* */, "CLOSURE ", closure},      // R(A) := closure(KPROTO[Bx])
        opcode{0, 1, OpArgU, OpArgN, IABC /* */, "VARARG  ", vararg},      // R(A), R(A+1), ..., R(A+B-2) = vararg
        opcode{0, 0, OpArgU, OpArgU, IAx /*  */, "EXTRAARG", nullptr},      // extra (larger) argument for previous opcode
};