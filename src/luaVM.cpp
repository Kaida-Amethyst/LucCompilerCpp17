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
//    std::cerr << "stack->pc = " << stack->pc << " ";
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
//        std::cerr << "Fetch : " << f << " ";
        auto inst = new Instruction(f);
//        std::cerr << "Execute " << inst->OpName() << std::endl;
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

void LuaVM::callExClosure(int nArgs, int nResults, luaClosure *c) {
    auto newStack = new luaStack(nArgs + 20);
    newStack->closure = c;
    auto args = this->stack->popN(nArgs);
    newStack->pushN(args, nArgs);
    this->stack->pop();
    this->pushLuaStack(newStack);
    auto r = c->func(this);
    this->popLuaStack();
    if (nResults != 0){
        auto results = newStack->popN(r);
        this->stack->check(static_cast<int>(results->size()));
        this->stack->pushN(results, nResults);
    }
}

int LuaVM::Load(byte * chunk, const std::string& chunkName, char mode) {
    auto proto = Undump(chunk);
    auto c = new luaClosure(proto);
    if (proto->Upvalues != nullptr && !proto->Upvalues->empty()){
        auto & env = this->registry->get(luaValue(LUA_RIDX_GLOBALS));
        c->upvals->at(0) = &env;
    }
    this->stack->emplace(c);
    return 0;
}

void LuaVM::Call(int nArgs, int nResults) {
//    for(auto i=0;i<stack->top;i++){
//        std::cout << "val.type() = " << stack->slots->at(i).type() << " val = "  << stack->slots->at(i) << std::endl;
//    }
//    std::cout << std::endl;
    auto val = stack->get(-(nArgs +1));
    if (val.type() == luaValue::Closure){
        luaClosure * c = val.get<luaValue::Closure>();
//        std::cout << "call " << c->proto->Source
//                  << "<" << c->proto->LineDefined
//                  << ", "<< c->proto->LastLineDefined
//                  << ">" << std::endl;
        if (c->proto != nullptr){
            this->callLuaClosure(nArgs, nResults, c);
        } else {
            this->callExClosure(nArgs, nResults, c);
        }
    }else{
        std::cerr << "Not a Fucntion! Location: luaVM.cpp, val.type() = " << val.type() << std::endl;
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
    auto subProto = stack->closure->proto->Protos->at(idx);
    auto closure = new luaClosure(subProto);
    for(auto i = 0; i < subProto->Upvalues->size();i++){
        auto & uvInfo = subProto->Upvalues->at(i);
        auto uvIdx = static_cast<int>(uvInfo.Idx);
        if (uvInfo.Instack == 1){
            if (stack->openuvs == nullptr){
                stack->openuvs = new std::unordered_map<int, luaValue*>();
            }
            if (stack->openuvs->find(uvIdx) != stack->openuvs->end()){
                closure->upvals->at(i) = stack->openuvs->at(uvIdx);
            }else{
                closure->upvals->at(i) = &stack->slots->at(uvIdx);
                stack->openuvs->insert(std::pair{uvIdx, closure->upvals->at(i)});
            }
        }else{
            closure->upvals->at(i) = stack->closure->upvals->at(uvIdx);
        }
    }
    stack->push(luaValue(closure));
//    auto proto = this->stack->closure->proto->Protos->at(idx);
//    this->stack->emplace(proto);
}

void LuaVM::CloseUpvalues(int a) {
    for(auto & [i, openuv] : *this->stack->openuvs){
        if (i >= a-1){
//            auto val = *openuv;
//            openuv = &val;
            this->stack->openuvs->erase(i);
        }
    }
}