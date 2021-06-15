//
// Created by ziyue on 2021/6/14.
//

#include "luaVM.h"
#include "instruction.h"
#include "opcodes.h"



int LuaVM::PC() const {
    return stack->pc;
}

void LuaVM::AddPC(int n) {
    stack->pc+=n;
}

uint32_t LuaVM::Fetch() {
    std::cerr << "stack->pc = " << stack->pc << " ";
    auto i = stack->closure->proto->Code->at(stack->pc);
    stack->pc++;
    return i;
}

void LuaVM::GetConst(int idx) {
    auto c = this->stack->closure->proto->Constants->at(idx);
    this->stack->push(c);
}

void LuaVM::GetRK(int rk) {
    if (rk > 0xFF){
        GetConst(rk & 0xFF);
    }else{
        PushValue(rk+1);
    }
}

void LuaVM::runLuaClosure() {
    while(true){
        auto f = this->Fetch();
        std::cerr << "Fetch : " << f << " ";
        auto inst = new Instruction(f);
        std::cerr << "Execute " << inst->OpName() << std::endl;
        inst->Execute(*this);
        if (inst->Opcode() == OP_RETURN){
            break;
        }
    }
}

void LuaVM::callLuaClosure(int nArgs, int nResults, luaClosure * c) {
    if (c == nullptr || c->proto == nullptr){
        std::cerr << "clousre is nullptr!, location : callLuaClosure" << std::endl;
    }
    auto nRegs = static_cast<int>(c->proto->MaxStackSize);
    auto nParams = static_cast<int>(c->proto->NumParams);
    auto isVararg = (c->proto->IsVararg == 1);
    auto newStack = new luaStack(nRegs + 20);
    newStack->closure = c;
    auto funcAndArgs = this->stack->popN(nArgs+1);
    funcAndArgs->erase(funcAndArgs->begin());
    newStack->pushN(funcAndArgs, nParams);
    newStack->top = nRegs;
    if (nArgs > nParams && isVararg){
        newStack->varargs = new std::vector<luaValue>();
        for(auto iter = funcAndArgs->begin() + nParams + 1;iter!= funcAndArgs->end();iter++){
            newStack->varargs->push_back(*iter);
        }
    }
    this->pushLuaStack(newStack);
    this->runLuaClosure();
    this->popLuaStack();
    if (nResults != 0){
        auto results = newStack->popN(newStack->top - nRegs);
        this->stack->check(static_cast<int>(results->size()));
        this->stack->pushN(results, nResults);
    }
}

int LuaVM::Load(byte * chunk, const std::string& chunkName, char mode) {
    auto proto = Undump(chunk);
//    luaValue c(proto);
//    c.CreateClosure(proto);
//    std::cerr << "After c.CreateClosure, c.type = " << c.type() << std::endl;
    this->stack->emplace(proto);
    return 0;
}

void LuaVM::Call(int nArgs, int nResults) {
    auto & val = stack->get(-(nArgs +1));
    if (val.type() == luaValue::Closure){
        luaClosure * c = val.get<luaValue::Closure>();
        std::cout << "call " << c->proto->Source
                  << "<" << c->proto->LineDefined
                  << ", "<< c->proto->LastLineDefined
                  << ">" << std::endl;
        this->callLuaClosure(nArgs, nResults, c);
    }else{
        std::cerr << "Not a Fucntion! Location: lua_vm.cpp, val.type() = " << val.type() << std::endl;
        abort();
    }
}

int LuaVM::RegisterCount() {
    return static_cast<int>(this->stack->closure->proto->MaxStackSize);
}

void LuaVM::LoadVararg(int n) {
    if (n < 0){
        n = static_cast<int>(this->stack->varargs->size());
    }
    this->stack->check(n);
    this->stack->pushN(this->stack->varargs, n);
}

void LuaVM::LoadProto(int idx) {
    auto proto = this->stack->closure->proto->Protos->at(idx);
    this->stack->emplace(proto);
//    auto clousre = new luaValue();
//    clousre->CreateClosure(proto);
//    this->stack->push(clousre);
}