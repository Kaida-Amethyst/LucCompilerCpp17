//
// Created by ziyue on 2021/6/14.
//

#include "luaVM.h"
#include "instruction.h"
#include "opcodes.h"
#include "assistants.h"
//#include "../compile.h"
#include "parser.h"
#include "codeGen.h"

static bool debug = false;

using namespace std::string_literals;


Prototype * Compile(const std::string & chunkName, const std::string & chunk){
    auto ast = parse(chunk, chunkName);
    return GenProto(ast);
}

int LuaVM::PC() const {
    return stack->pc;
}

void LuaVM::AddPC(int n) {
    stack->pc+=n;
}

uint32_t LuaVM::Fetch() {
    if (debug) std::cerr << "stack->pc = " << stack->pc << " ";
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

void LuaVM::Arith(ArithOp op) {
    luaValue a, b;
    b = stack->pop();
    if (op != LUA_OPUNM && op != LUA_OPBNOT){
        a = stack->pop();
    }else{
        a = b;
    }

    auto opera = Operators[op];
    if (auto result = __arith(a, b, opera); result.type() != luaValue::Nil){
        stack->push(result);
        return ;
    }
    auto mm = opera.metamethod;

    if (auto [result, ok] = callMetaMethod(a, b, mm, *this); ok){
        this->stack->push(result);
        return ;
    }

    std::cerr << "arithmetic error! Location : LuaVM::Arith" << std::endl;
}

bool LuaVM::__eq(const luaValue &a, const luaValue &b){
    switch (a.type()) {
        case luaValue::Nil:
            return b.type() == luaValue::Nil;
        case luaValue::Int:
            if(b.type() == luaValue::Int){
                return a.get<luaValue::Int>() == b.get<luaValue::Int>();
            }else if(b.type() == luaValue::Float){
                return a.get<luaValue::Int>() == b.get<luaValue::Float>();
            }else{
                return false;
            }
        case luaValue::Float:
            if(b.type() == luaValue::Int){
                return a.get<luaValue::Float>() == static_cast<double>(b.get<luaValue::Int>());
            }else if(b.type() == luaValue::Float){
                return a.get<luaValue::Float>() == b.get<luaValue::Float>();
            }else{
                return false;
            }
        case luaValue::Boolean:
            return b.type() == luaValue::Boolean && a.get<luaValue::Boolean>() == b.get<luaValue::Boolean>();
        case luaValue::String:
            return b.type() == luaValue::String && a.get<luaValue::String>() == b.get<luaValue::String>();
        case luaValue::Table:
            if (b.type() == luaValue::Table && !(a == b)){
                if (auto [result, ok] = callMetaMethod(a, b, "__eq", *this); ok){
                    return asf::convertToBoolen(result);
                }
            }
            return a == b;
        default:
            return a == b;
//            std::cerr << "Unknown lua type. Location: lua_state::__eq" << std::endl;
//            abort();
    }
}

bool LuaVM::__lt(const luaValue &a, const luaValue &b){
    if(a.type() == luaValue::String){
        if (b.type() == luaValue::String){
            return a.get<luaValue::String>() < b.get<luaValue::String>();
        }
    }else if(a.type() == luaValue::Int){
        if (b.type() == luaValue::Int){
            return a.get<luaValue::Int>() < b.get<luaValue::Int>();
        }else if(b.type() == luaValue::Float){
            return a.get<luaValue::Int>() < b.get<luaValue::Float>();
        }
    }else if(a.type() == luaValue::Float){
        if (b.type() == luaValue::Int){
            return a.get<luaValue::Float>() < b.get<luaValue::Int>();
        }else if(b.type() == luaValue::Float){
            return a.get<luaValue::Float>() < b.get<luaValue::Float>();
        }
    }

    if (auto [result, ok] = callMetaMethod(a,b,"__lt", *this); ok){
        return asf::convertToBoolen(result);
    }else{
        std::cerr << "Comparison error! location: LuaVM::__lt." << std::endl;
        abort();
    }
}

bool LuaVM::__le(const luaValue &a, const luaValue &b){
    if(a.type() == luaValue::String){
        if (b.type() == luaValue::String){
            return a.get<luaValue::String>() <= b.get<luaValue::String>();
        }
    }else if(a.type() == luaValue::Int){
        if (b.type() == luaValue::Int){
            return a.get<luaValue::Int>() <= b.get<luaValue::Int>();
        }else if(b.type() == luaValue::Float){
            return a.get<luaValue::Int>() <= b.get<luaValue::Float>();
        }
    }else if(a.type() == luaValue::Float){
        if (b.type() == luaValue::Int){
            return a.get<luaValue::Float>() <= b.get<luaValue::Int>();
        }else if(b.type() == luaValue::Float){
            return a.get<luaValue::Float>() <= b.get<luaValue::Float>();
        }
    }

    if (auto [result, ok] = callMetaMethod(a, b, "__le", *this); ok){
        return asf::convertToBoolen(result);
    }else if (auto [result, ok] = callMetaMethod(b, a, "__lt", *this); ok){
        return !(asf::convertToBoolen(result));
    }else{
        std::cerr << "Comparison error! location: LuaVM::__lt." << std::endl;
        abort();
    }
}

LuaType LuaVM::__getTable(const luaValue& t, const luaValue& k, bool raw){
    if (t.type() == luaValue::Table){
        luaTable * tbl = t.get<luaValue::Table>();
        //|| tbl->find(k)
        if (raw  || !tbl->hasMetafield("__index")){
            auto v = tbl->get(k);
            this->stack->push(v);
            return typeOf(v);
//            if (tbl->find(k)){
//                auto v = tbl->get(k);
//                this->stack->push(v);
//                return typeOf(v);
//            }
        }
//        auto v = tbl->get(k);
//        if (raw || !(v == nil) || !tbl->hasMetafield("__index")){
//            this->stack->push(v);
//            return typeOf(v);
//        }
    }
    if (!raw){
        if (auto mf = getMetafield(t, "__index", *this); !(mf == nil)) {
            switch (mf.type()) {
                case luaValue::Table:
                    return this->__getTable(mf, k, false);
                case luaValue::Closure:
                    this->stack->push(mf);
                    this->stack->push(t);
                    this->stack->push(k);
                    this->Call(2, 1);
                    auto v = this->stack->get(-1);
                    return typeOf(v);
            }
        }
    }
    std::cerr << "index error!, Location: LuaVM: GetTable" << std::endl;
    abort();
}

void LuaVM::__setTable(const luaValue& t, luaValue k, luaValue v, bool raw){
    if (t.type() == luaValue::Table){
        auto * tbl = t.get<luaValue::Table>();
        if (raw || !tbl->hasMetafield("__newindex")){
            tbl->put(k, v);
            return ;
        }
//        if (raw ||  tbl->find(k) || !tbl->hasMetafield("__newindex")){
//            tbl->put(k, v);
//            return ;
//        }
    }
    if (!raw){
        if (auto mf = getMetafield(t, "__newindex", *this); !(mf == nil)){
            switch (mf.type()) {
                case luaValue::Table:
                    this->__setTable(mf, k, v, false);
                    return ;
                case luaValue::Closure:
                    this->stack->push(mf);
                    this->stack->push(t);
                    this->stack->push(k);
                    this->stack->push(v);
                    this->Call(3, 0);
                    return ;
            }
        }
    }
    std::cerr << "index error! Location: LuaVM::__setTable" << std::endl;
    abort();
}

bool LuaVM::Compare(int idx1, int idx2, CompareOp op){
    if (!this->stack->isValid(idx1) || !this->stack->isValid(idx2)){
        return false;
    }

    auto a{stack->get(idx1)};
    auto b{stack->get(idx2)};
    switch (op) {
        case LUA_OPEQ: return __eq(a, b);
        case LUA_OPLT: return __lt(a, b);
        case LUA_OPLE: return __le(a, b);
        default:
            std::cerr << "Invalid Compare Op! Location: luaState::Compare" << std::endl;
            abort();
    }
}

void LuaVM::Len(int idx){
    auto val{stack->get(idx)};
    if(val.type() == luaValue::String){
        stack->emplace(static_cast<int64_t>(val.get<luaValue::String>().size()));
    }else if (auto [result, ok] = callMetaMethod(val, val, "__len", *this); ok){
        this->stack->push(result);
    }else if (val.type() == luaValue::Table){
        stack->emplace(static_cast<int64_t>(val.get<luaValue::Table>()->len()));
    }else{
        std::cerr << "Not a String or Table! No MetaMethod. Location: LuaVM::Len" << std::endl;
        abort();
    }
}

void LuaVM::Concat(int n){
    if (n == 0) {
        stack->emplace(""s);
    }else if(n>=2){
        for(int i=1;i<n;i++){
            if (IsString(-1) && IsString(-2)){
                auto s2{ToString(-1)};
                auto s1{ToString(-2)};
                stack->pop();
                stack->pop();
                stack->emplace(s1+s2);
                continue;
            }

            auto b = this->stack->pop();
            auto a = this->stack->pop();
            if (auto [result, ok] = callMetaMethod(a, b, "__concat", *this); ok){
                this->stack->push(result);
                continue;
            }

            std::cerr << "Not a String! Or no metaMethod! Location: luaState::Concat" << std::endl;
            abort();
        }
    }
}

LuaType LuaVM::GetTable(int idx){
    auto t = stack->get(idx);
    auto k = stack->pop();
    return __getTable(t, k, false);
}

void LuaVM::runLuaClosure() {
    while(true){
        auto f = this->Fetch();
        if (debug) std::cerr << "Fetch : " << f << " ";
        auto inst = new Instruction(f);
        if (debug) std::cerr << "Execute " << inst->OpName() << std::endl;
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

int LuaVM::Load(const std::string & chunk, const std::string& chunkName){
    auto proto = Compile(chunkName, chunk);
    auto c = new luaClosure(proto);
    if (proto->Upvalues != nullptr && !proto->Upvalues->empty()){
        auto & env = this->registry->get(luaValue(LUA_RIDX_GLOBALS));
        c->upvals->at(0) = &env;
    }
    this->stack->emplace(c);
    return 0;
}

int LuaVM::Load(byte * chunk, const std::string& chunkName) {
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
//    auto ok = val.type() == luaValue::Closure;
    if (val.type() != luaValue::Closure){
        if (auto mf = getMetafield(val, "__call", *this); !(mf == nil) ){
            if (mf.type() == luaValue::Closure){
                this->stack->push(val);
                this->Insert(-(nArgs + 2));
                nArgs += 1;
                auto c = mf.get<luaValue::Closure>();
                if (c->proto != nullptr){
                    this->callLuaClosure(nArgs, nResults, c);
                } else {
                    this->callExClosure(nArgs, nResults, c);
                }
                return;
//                ok = !ok;
            }
        }
    }

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
    } else {
        std::cerr << "Not a Fucntion! Location: luaVM.cpp, val.type() = " << val.type()  << " ";
        if (val.type() <= 5){
            std::cerr << "val = " << val;
        }
        std::cerr << std::endl;
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

LuaType LuaVM::GetField(int idx, std::string k){
    auto t = stack->get(idx);
    return __getTable(t, luaValue(std::move(k)), false);
}

LuaType LuaVM::GetI(int idx, int64_t i){
    auto t = stack->get(idx);
    return __getTable(t, luaValue(i), false);
}

void LuaVM::SetTable(int idx){
    auto t = stack->get(idx);
    auto v = stack->pop();
    auto k = stack->pop();
    __setTable(t,k,v, false);
    return ;
}

void LuaVM::SetField(int idx, const std::string& k){
    auto t = stack->get(idx);
    auto v = stack->pop();
    __setTable(t, luaValue(k), v, false);
}

void LuaVM::SetI(int  idx, int64_t i){
    auto t = stack->get(idx);
    auto v = stack->pop();
    __setTable(t, luaValue(i), v, false);
}

bool LuaVM::GetMetatable(int idx) {
    auto val = this->stack->get(idx);
    if (auto mt = val.getMetaTable(*this); mt != nullptr){
        this->stack->emplace(mt);
        return true;
    }else{
        return false;
    }
}

void LuaVM::SetMetatable(int idx){
    auto val = this->stack->get(idx);
    auto mtVal = this->stack->pop();
    if (mtVal == nil){
        val.setMetaTable(nullptr, *this);
    }else if(mtVal.type() == luaValue::Table){
        val.setMetaTable(mtVal.get<luaValue::Table>(), *(this));
    }else{
        std::cerr << "table expected! Location: LuaVM::SetMetatable" << std::endl;
    }

    this->stack->set(idx, val);
}

uint32_t LuaVM::RawLen(int idx){
    auto val{stack->get(idx)};
    if(val.type() == luaValue::String){
        return static_cast<uint32_t>(val.get<luaValue::String>().size());
    }
    else if (val.type() == luaValue::Table){
        return static_cast<uint32_t>(val.get<luaValue::Table>()->len());
    }
    else{
        return 0;
    }
}

bool LuaVM::RawEqual(int idx1, int idx2) {
    if (!this->stack->isValid(idx1) || !this->stack->isValid(idx2)){
        return false;
    }

    auto a = this->stack->get(idx1);
    auto b = this->stack->get(idx2);
    return __eq(a, b);
}

LuaType LuaVM::RawGet(int idx){
    auto t = stack->get(idx);
    auto k = stack->pop();
    return __getTable(t, k, true);
}



void LuaVM::RawSet(int idx) {
    auto t = stack->get(idx);
    auto v = stack->pop();
    auto k = stack->pop();
    __setTable(t,k,v, true);
    return ;
}

LuaType LuaVM::RawGetI(int idx, int64_t i) {
    auto t = stack->get(idx);
    return __getTable(t, luaValue(i), true);
}

void LuaVM::RawSetI(int idx, int64_t i) {
    auto t = stack->get(idx);
    auto v = stack->pop();
    __setTable(t, luaValue(i), v, true);
}

bool LuaVM::Next(int idx) {
    auto val = this->stack->get(idx);
    if (val.type() == luaValue::Table){
        auto t = val.get<luaValue::Table>();
        auto key = this->stack->pop();
        //&& t->find(nextKey)
        if (auto nextKey = t->nextKey(key); !(nextKey == nil) ){
            this->stack->push(nextKey);
            this->stack->push(t->get(nextKey));
            return true;
        }
        return false;
    }
    std::cerr << "Table Expected: Location: LuaVM::Next" << std::endl;
    abort();
}

