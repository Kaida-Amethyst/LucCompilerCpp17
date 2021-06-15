//
// Created by ziyue on 2021/6/13.
//

#include "instruction.h"
#include "opcodes.h"
#include "assistants.h"

const int MAXARG_Bx = (1<<18) - 1;
const int MAXARG_sBx = MAXARG_Bx >> 1;

const int debug = 1;

Instruction::Instruction(uint32_t code):proto_code{code} {}

int Instruction::Opcode() {
    return static_cast<int>(proto_code & 0x3F);
}

std::tuple<int, int, int> Instruction::ABC() {
    return std::tuple{
            static_cast<int>((proto_code >> 6) & 0xFF),
            static_cast<int>((proto_code >> 23) & 0x1FF),
            static_cast<int>((proto_code >> 14) & 0x1FF),
    };
}

std::pair<int, int> Instruction::ABx() {
    return std::pair{
            static_cast<int>((proto_code >> 6) & 0xFF),
            static_cast<int>(proto_code >> 14)
    };
}

std::pair<int, int> Instruction::AsBx() {
    return std::pair{
            static_cast<int>((proto_code >> 6) & 0xFF),
            static_cast<int>(proto_code >> 14) - MAXARG_sBx
    };
}

int Instruction::Ax() {
    return static_cast<int>(proto_code >> 6);
}

std::string Instruction::OpName() {
    return opcodes[Opcode()].name;
}

byte Instruction::OpMode() {
    return opcodes[Opcode()].opMode;
}

byte Instruction::BMode() {
    return opcodes[Opcode()].argBMode;
}

byte Instruction::CMode() {
    return opcodes[Opcode()].argCMode;
}

void Instruction::Execute(LuaVM &vm) {

    auto action = opcodes.at(Opcode()).action;
    if (action != nullptr){
        action(*this, vm);
    }else{
        std::cerr << OpName() << "Location: Instruction.cpp" << std::endl;
        abort();
    }

//    std::cerr << "vm stack slots: " ;
//    for(auto i=0;i<vm.stack->top;i++){
//        std::cerr << "[" <<  vm.stack->slots->at(i) << "]";
//    }
//    std::cerr << std::endl;
}


void move(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++; b++;
    vm.Copy(b, a);
}

void jmp(Instruction & i, LuaVM & vm){
    auto [a, sBx] = i.AsBx();
    vm.AddPC(sBx);
    if (a != 0){
        vm.CloseUpvalues(a);
        abort();
    }
}

void loadNil(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++;
    vm.PushNil();
    for(int j=a;j<=a+b;j++){
        vm.Copy(-1, j);
    }
    vm.Pop(1);
}

void loadBool(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++;
    vm.PushBoolean(b!=0);
    vm.Replace(a);
    if (c!=0){
        vm.AddPC(1);
    }
}

void loadK(Instruction & i, LuaVM & vm){
    auto [a, bx] = i.ABx();
    a++;
    vm.GetConst(bx);
    vm. Replace(a);
}

void loadKx(Instruction & i, LuaVM & vm){
    auto [a, b] = i.ABx();
    a++;
    auto ax = Instruction(vm.Fetch()).Ax();
    vm.GetConst(ax);
    vm.Replace(a);
}

void __binaryArith(Instruction & i, LuaVM & vm, ArithOp op){
    auto [a,b,c] = i.ABC();
    a++;
    vm.GetRK(b);
    vm.GetRK(c);
    vm.Arith(op);
    vm.Replace(a);
}

void __unaryArith(Instruction & i, LuaVM & vm, ArithOp op){
    auto [a,b,c] = i.ABC();
    a++;b++;
    vm.PushValue(b);
    vm.Arith(op);
    vm.Replace(a);
}

void add (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPADD ); };  // +
void sub (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPSUB ); };  // -
void mul (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPMUL ); };  // *
void mod (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPMOD ); }; // %
void Pow (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPPOW ); }; // ^
void Div (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPDIV ); }; // /
void idiv(Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPIDIV); }; // //
void band(Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPBAND); }; // &
void bor (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPBOR ); }; // |
void bxor(Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPBXOR); }; // ~
void shl (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPSHL ); }; // <<
void shr (Instruction & i, LuaVM & vm)  { __binaryArith(i, vm, LUA_OPSHR ); }; // >>
void unm (Instruction & i, LuaVM & vm)  { __unaryArith (i, vm, LUA_OPUNM ); }; // -
void bnot(Instruction & i, LuaVM & vm)  { __unaryArith (i, vm, LUA_OPBNOT); }; // ~


void __len(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++; b++;
    vm.Len(b);
    vm.Replace(a);
}

void concat(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++;b++;c++;
    auto n = c-b+1;
    vm.CheckStack(n);
    for(auto i=b;i<=c;i++){
        vm.PushValue(i);
    }
    vm.Concat(n);
    vm.Replace(a);
}

void __comare(Instruction & i, LuaVM & vm, CompareOp op){
    auto [a,b,c] = i.ABC();
    vm.GetRK(b);
    vm.GetRK(c);
    if (vm.Compare(-2,-1, op) != (a!=0)){
        vm.AddPC(1);
    }
    vm.Pop(2);
}

void eq(Instruction & i, LuaVM & vm){ __comare(i, vm, LUA_OPEQ); };
void lt(Instruction & i, LuaVM & vm){ __comare(i, vm, LUA_OPLT); };
void le(Instruction & i, LuaVM & vm){ __comare(i, vm, LUA_OPLE); };

void Not(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++; b++;
    vm.PushBoolean(!vm.ToBoolean(b));
    vm.Replace(a);
}

void testSet(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++; b++;
    if (vm.ToBoolean(b) == (c != 0)){
        vm.Copy(b, a);
    }else{
        vm.AddPC(1);
    }
}

void test(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++;
    if (vm.ToBoolean(a)!=(c!=0)){
        vm.AddPC(1);
    }
}

void length(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++; b++;
    vm.Len(b);
    vm.Replace(a);
}

void forPrep(Instruction & i, LuaVM & vm) {
    auto [a, sBx] = i.AsBx();
    a += 1;

    if (vm.Type(a) == LUA_TSTRING) {
        vm.PushNumber(vm.ToNumber(a));
        vm.Replace(a);
    }
    if (vm.Type(a+1) == LUA_TSTRING) {
        vm.PushNumber(vm.ToNumber(a + 1));
        vm.Replace(a + 1);
    }
    if (vm.Type(a+2) == LUA_TSTRING) {
        vm.PushNumber(vm.ToNumber(a + 2));
        vm.Replace(a + 2);
    }

    vm.PushValue(a);
    vm.PushValue(a + 2);
    vm.Arith(LUA_OPSUB);
    vm.Replace(a);
    vm.AddPC(sBx);
}

void forLoop(Instruction & i, LuaVM & vm) {
    auto [a, sBx] = i.AsBx();
    a += 1;
    vm.PushValue(a + 2);
    vm.PushValue(a);
    vm.Arith(LUA_OPADD);
    vm.Replace(a);
    auto isPositiveStep = vm.ToNumber(a+2) >= 0;
    if (isPositiveStep && vm.Compare(a, a+1, LUA_OPLE) || !isPositiveStep && vm.Compare(a+1, a, LUA_OPLE)) {
        vm.AddPC(sBx);
        vm.Copy(a, a+3);
    }
}

const int LFIELDS_PER_FLUSE = 50;

void newTable(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++;
    vm.CreateTable(asf::Fb2int(b), asf::Fb2int(c));
    vm.Replace(a);
}

void getTable(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++;b++;
    vm.GetRK(c);
//    std::cerr << "a = "<< a << " b = " << b << " c = " << (c&0xFF) << std::endl;
    vm.GetTable(b);
    vm.Replace(a);
}

void setTable(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++;
    vm.GetRK(b);
    vm.GetRK(c);
    vm.SetTable(a);
}

void setList (Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a++;
    if (c > 0) c--;
    else c = Instruction(vm.Fetch()).Ax();
    auto idx = static_cast<int64_t>(c*LFIELDS_PER_FLUSE);

    for(auto j=1;j<=b;j++){
        idx++;
        vm.PushValue(a+j);
        vm.SetI(a, idx);
    }
}


void closure(Instruction & i, LuaVM & vm){
    auto [a, bx] = i.ABx();
    a += 1;
    vm.LoadProto(bx);
    vm.Replace(a);
}

void call(Instruction & i, LuaVM & vm){
    auto [a, b, c] = i.ABC();
    a += 1;
    auto nArgs = _pushFuncAndArgs(a,b,vm);
    vm.Call(nArgs, c-1);
    _popResults(a,c,vm);
}

int _pushFuncAndArgs(int a, int b, LuaVM & vm){
    if (b >= 1){
        vm.CheckStack(b);
        for(int i=a;i<a+b;i++){
            vm.PushValue(i);
        }
        return b-1;
    }else{
        _fixStack(a, vm);
        return vm.GetTop() - vm.RegisterCount() - 1;
    }
}

void _fixStack(int a, LuaVM & vm){
    auto x = static_cast<int>(vm.ToInteger(-1));
    vm.Pop(1);
    vm.CheckStack(x - a);
    for(auto i = a; i < x ; i++){
        vm.PushValue(i);
    }
    vm.Rotate(vm.RegisterCount()+1, x - a);
}

void _return(Instruction & i, LuaVM & vm){
    auto [a, b, _] = i.ABC();
    a += 1;
    if (b == 1){

    }else if (b > 1){
        vm.CheckStack(b-1);
        for (auto idx=a;idx <= a+b-2;idx++){
            vm.PushValue(idx);
        }
    }else{
        _fixStack(a, vm);
    }
}

void vararg(Instruction & i, LuaVM & vm){
    auto [a,b,_] = i.ABC();
    a += 1;
    if (b != 1){
        vm.LoadVararg(b-1);
        _popResults(a, b, vm);
    }
}

void tailCall(Instruction & i, LuaVM & vm){
    auto [a,b,_] = i.ABC();
    a += 1;
    auto c = 0;
    auto nArgs = _pushFuncAndArgs(a,b,vm);
    vm.Call(nArgs, -1);
    _popResults(a, c, vm);
}

void self(Instruction & i, LuaVM & vm){
    auto [a,b,c] = i.ABC();
    a ++; b++;
    vm.Copy(b, a+1);
    vm.GetRK(c);
    vm.GetTable(b);
    vm.Replace(a);
}

void _popResults(int a, int c, LuaVM & vm){
    if (c == 1){

    }else if (c > 1){
        for(auto i=a+c-2;i>=a;i--){
            vm.Replace(i);
        }
    }else{
        vm.CheckStack(1);
        vm.PushInteger(static_cast<int64_t>(a));
    }
}



static inline int LuaUpvalueIndex(int i){
    return LUA_REGISTRYINDEX - i;
}

void getUpVal(Instruction & i, LuaVM & vm){
    auto [a,b,_] = i.ABC();
    a++; b++;
    vm.Copy(LuaUpvalueIndex(b), a);
}

void setUpVal(Instruction & i, LuaVM & vm){
    auto [a, b, _] = i.ABC();
    a++; b++;
    vm.Copy(a, LuaUpvalueIndex(b));
}

void getTabUp(Instruction & i, LuaVM & vm){
    auto [a,b, c] = i.ABC();
    a ++; b++;
    vm.GetRK(c);
    vm.GetTable(LuaUpvalueIndex(b));
    vm.Replace(a);
}

void setTabUp(Instruction & i, LuaVM & vm){
    auto [a,b, c] = i.ABC();
    a++;
    vm.GetRK(b);
    vm.GetRK(c);
//    std::cerr << "using setTabUp, vm stack slots: " ;
//    for(auto i=0;i<vm.stack->top;i++){
//        std::cerr << "[" <<  vm.stack->slots->at(i) << "]";
//    }
    std::cerr << std::endl;
    vm.SetTable(LuaUpvalueIndex(a));
}