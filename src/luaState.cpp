//
// Created by ziyue on 2021/6/13.
//

#include "luaState.h"
#include "assistants.h"
#include "luaTable.h"
#include <iostream>
#include <string>
#include <utility>

using namespace std::string_literals;

luaState::luaState():stack{ new luaStack(20)}, registry{new luaTable(0, 0)}{
    registry->put(luaValue(LUA_RIDX_GLOBALS), luaValue(new luaTable(0,0)));
    this->pushLuaStack(new luaStack(LUA_MINSTACK));
};

int luaState::GetTop() const{
    return stack->top;
}

int luaState::AbsIndex(int idx) const {
    return stack->absIndex(idx);
}

bool luaState::CheckStack(int n) {
    stack->check(n);
    return true;
}

void luaState::Pop(int n) {
    while(n > 0){
        stack->pop(); n--;
    }
}

void luaState::Copy(int fromIdx, int toIdx) {
    auto val = stack->get(fromIdx);
    stack->set(toIdx, val);
}

void luaState::PushValue(int idx) {
    auto val = stack->get(idx);
    stack->push(val);
}

void luaState::Replace(int idx) {
    auto val = stack->pop();
    stack->set(idx, val);
}

void luaState::Insert(int idx) {
    Rotate(idx, 1);
}

void luaState::Remove(int idx) {
    Rotate(idx, -1);
    Pop(1);
}

void luaState::Rotate(int idx, int n) {
    auto t = stack->top - 1;
    auto p = stack->absIndex(idx) - 1;
    int m = n>=0 ? t - n:p - n - 1;

    stack->reverse(p, m);
    stack->reverse(m+1, t);
    stack->reverse(p, t);
}

void luaState::SetTop(int idx) {
    auto newTop{stack->absIndex(idx)};
    if (newTop < 0){
        std::cerr << "Stack Underflow. Location: api/lua_state. Method: SetTop()" << std::endl;
    }
    auto n{stack->top - newTop};
    while(n > 0){
        stack->pop(); n--;
    }
    while(n < 0){
        stack->emplace(nil); n++;
    }
}

void luaState::PushNil() {stack->emplace(nil);}

void luaState::PushBoolean(bool b) {stack->emplace(b);}

void luaState::PushInteger(int64_t n) {stack->emplace(n);}

void luaState::PushNumber(double n) {stack->emplace(n);}

void luaState::PushString(const std::string& s) {stack->emplace(s);}

std::string luaState::TypeName(LuaType tp) {
    switch (tp) {
        case LUA_TNONE:
            return "no value";
        case LUA_TNIL:
            return "nil";
        case LUA_TBOOLEAN:
            return "boolean";
        case LUA_TNUMBER:
            return "number";
        case LUA_TSTRING:
            return "string";
        case LUA_TTABLE:
            return "table";
        case LUA_TFUNCTION:
            return "function";
        case LUA_TTHREAD:
            return "thread";
        default:
            return "userdata";
    }
}

LuaType luaState::Type(int idx) const {
    if (stack->isValid(idx)){
        auto val{stack->get(idx)};
        return typeOf(val);
    }
    return LUA_TNONE;
}

bool luaState::IsNone(int idx) const{
    return Type(idx) == LUA_TNONE;
}

bool luaState::IsNil(int idx) const{
    return Type(idx) == LUA_TNIL;
}

bool luaState::IsNoneOrNil(int idx) const{
    return Type(idx) == LUA_TNONE || Type(idx) == LUA_TNIL;
}

bool luaState::IsBoolean(int idx) const{
    return Type(idx) == LUA_TBOOLEAN;
}

bool luaState::IsString(int idx) const {
    auto t{Type(idx)};
    return t == LUA_TSTRING || t == LUA_TNUMBER;
}

bool luaState::IsNumber(int idx) const {
    auto [_, ok] = ToNumberX(idx);
    return ok;
}

bool luaState::IsInteger(int idx) const {
    auto val{stack->get(idx)};
    if (val.index() == luaValue::Int)return true;
    if (val.index() == luaValue::String){
        return asf::isInteger(val.get<luaValue::String>());
    }
    return false;
}

bool luaState::ToBoolean(int idx) const{
    auto val{stack->get(idx)};
    return asf::convertToBoolen(val);
}

double luaState::ToNumber(int idx) const{
    auto [n, _] = ToNumberX(idx);
    return n;
}

std::pair<double, bool> luaState::ToNumberX(int idx) const {
    auto val{stack->get(idx)};
    switch (val.type()) {
        case luaValue::Int:
            return std::pair<double, bool>{val.get<luaValue::Int>(), true};
        case luaValue::Float:
            return std::pair<double, bool>{val.get<luaValue::Float>(), true};
        default:
            return std::pair<double, bool>{0, false};
    }
}

int64_t luaState::ToInteger(int idx) const {
    auto [n, _] = ToIntegerX(idx);
    return n;
}


std::pair<int64_t, bool> luaState::ToIntegerX(int idx) const {
    auto val{stack->get(idx)};
    if (val.type() == luaValue::Int){
        return std::pair<int64_t, bool>{val.get<luaValue::Int>(), true};
    }
    return std::pair{0, false};
}

std::pair<std::string, bool> luaState::ToStringX(int idx){
    auto val{stack->get(idx)};
    switch(val.type()){
        case luaValue::Int:{
            std::string s{std::to_string(val.get<luaValue::Int>())};
            stack->set_v(idx, s);
            return std::pair{s, true};
        }
        case luaValue::Float:{
            std::string s{std::to_string(val.get<luaValue::Float>())};
            stack->set_v(idx, s);
            return std::pair{s, true};
        }
        case luaValue::String:
            return std::pair{val.get<luaValue::String>(), true};
        default:
            return std::pair{"", false};
    }
}

std::string luaState::ToString(int idx) {
    auto [s, _] = ToStringX(idx);
    return s;
}

luaValue luaState::__arith(luaValue &a, luaValue &b, Operator &op) {
    if (op.FloatFunc == nullptr){
        auto [x, a_ok] = asf::convertToInteger(a);
        if(a_ok){
            auto [y, b_ok] = asf::convertToInteger(b);
            if (b_ok){
                return luaValue(op.IntegerFunc(x, y));
            }
        }
    }else{
        if (op.IntegerFunc != nullptr){
            if (a.type() == luaValue::Int && b.type() == luaValue::Int){
                return luaValue(op.IntegerFunc(a.get<luaValue::Int>(), b.get<luaValue::Int>()));
            }
        }else{
            auto [x , a_ok] = asf::convertToFloat(a);
            if (a_ok){
                auto [y, b_ok] = asf::convertToFloat(b);
                return luaValue(op.FloatFunc(x, y));
            }
        }
    }
    return luaValue(nil);
}



bool luaState::__eq(const luaValue &a, const luaValue &b) const {
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
                return a.get<luaValue::Float>() == b.get<luaValue::Int>();
            }else if(b.type() == luaValue::Float){
                return a.get<luaValue::Float>() == b.get<luaValue::Float>();
            }else{
                return false;
            }
        case luaValue::Boolean:
            return b.type() == luaValue::Boolean && a.get<luaValue::Boolean>() == b.get<luaValue::Boolean>();
        case luaValue::String:
            return b.type() == luaValue::String && a.get<luaValue::String>() == b.get<luaValue::String>();
        default:
            std::cerr << "Unknown lua type. Location: lua_state::__eq" << std::endl;
            abort();
    }
}

bool luaState::__lt(const luaValue &a, const luaValue &b) const {
    if(a.type() == luaValue::String){
        if (b.type() == luaValue::String){
            return a.get<luaValue::String>() < b.get<luaValue::String>();
        }else{
            std::cerr << "Didn't implement __lt for string and non-string, location: lua_state::__lt." << std::endl;
        }
    }else if(a.type() == luaValue::Int){
        if (b.type() == luaValue::Int){
            return a.get<luaValue::Int>() < b.get<luaValue::Int>();
        }else if(b.type() == luaValue::Float){
            return a.get<luaValue::Int>() < b.get<luaValue::Float>();
        }else{
            std::cerr << "Didn't implement __lt for int and non-(int/double), location: lua_state::__lt." << std::endl;
        }
    }else if(a.type() == luaValue::Float){
        if (b.type() == luaValue::Int){
            return a.get<luaValue::Float>() < b.get<luaValue::Int>();
        }else if(b.type() == luaValue::Float){
            return a.get<luaValue::Float>() < b.get<luaValue::Float>();
        }else{
            std::cerr << "Didn't implement __lt for int and non-(int/double), location: lua_state::__lt." << std::endl;
        }
    }else{
        std::cerr << "Comparison error! location: lua_state::__lt." << std::endl;
        abort();
    }
    return false;
}

bool luaState::__le(const luaValue &a, const luaValue &b) const {
    if(a.type() == luaValue::String){
        if (b.type() == luaValue::String){
            return a.get<luaValue::String>() <= b.get<luaValue::String>();
        }else{
            std::cerr << "Didn't implement __lt for string and non-string, location: lua_state::__le." << std::endl;
        }
    }else if(a.type() == luaValue::Int){
        if (b.type() == luaValue::Int){
            return a.get<luaValue::Int>() <= b.get<luaValue::Int>();
        }else if(b.type() == luaValue::Float){
            return a.get<luaValue::Int>() <= b.get<luaValue::Float>();
        }else{
            std::cerr << "Didn't implement __lt for int and non-(int/double), location: lua_state::__le." << std::endl;
        }
    }else if(a.type() == luaValue::Float){
        if (b.type() == luaValue::Int){
            return a.get<luaValue::Float>() <= b.get<luaValue::Int>();
        }else if(b.type() == luaValue::Float){
            return a.get<luaValue::Float>() <= b.get<luaValue::Float>();
        }else{
            std::cerr << "Didn't implement __lt for int and non-(int/double), location: lua_state::__le." << std::endl;
        }
    }else{
        std::cerr << "Comparison error! location: lua_state::__lt." << std::endl;
        abort();
    }
    return false;
}

void luaState::Arith(ArithOp op) {
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
    }else{
        std::cerr << "arithmetic error! Location : luaState::Arith" << std::endl;
    }

}


bool luaState::Compare(int idx1, int idx2, CompareOp op) {
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

void luaState::Len(int idx) {
    auto val{stack->get(idx)};
    if(val.type() == luaValue::String){
        stack->emplace(static_cast<int64_t>(val.get<luaValue::String>().size()));
    }
    else if (val.type() == luaValue::Table){
        stack->emplace(static_cast<int64_t>(val.get<luaValue::Table>()->len()));
    }
    else{
        std::cerr << "Not a String or Table! Location: luaState::Len" << std::endl;
        abort();
    }
}

void luaState::Concat(int n) {
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
            std::cerr << "Not a String! Location: luaState::Concat" << std::endl;
            abort();
        }
    }
}

void luaState::NewTable() {
    CreateTable(0, 0);
}

void luaState::CreateTable(int nArr, int nRec) {
    auto t = new luaTable(nArr, nRec);
    stack->emplace(t);
}


LuaType luaState::__getTable(const luaValue& t, const luaValue& k) {
    if (t.type() == luaValue::Table){
        auto tbl = t.get<luaValue::Table>();
        auto v = tbl->get(k);
        stack->push(v);
        return typeOf(v);
    }else{
        std::cerr << "Not a Table! Location: lua_state::getTable, t.type = " << t.type() << std::endl;
        abort();
    }
}

LuaType luaState::GetTable(int idx) {
    auto t = stack->get(idx);
    auto k = stack->pop();
    return __getTable(t, k);
}

LuaType luaState::GetField(int idx, std::string k) {
    auto t = stack->get(idx);
    return __getTable(t, luaValue(std::move(k)));
}

LuaType luaState::GetI(int idx, int64_t i) {
    auto t = stack->get(idx);
    return __getTable(t, luaValue(i));
}

void luaState::SetTable(int idx) {
    auto t = stack->get(idx);
    auto v = stack->pop();
    auto k = stack->pop();
    __setTable(t,k,v);
    return ;
}

void luaState::__setTable(const luaValue& t, luaValue k, luaValue v) {
    if (t.type() == luaValue::Table) {
        auto tbl = t.get<luaValue::Table>();
        tbl->put(std::move(k), std::move(v));
        return;
    }else{
        std::cerr << "Not a Table! Location: lua_state::setTable, t.type = "<< t.type() << std::endl;
        abort();
    }
}




void luaState::SetField(int idx, const std::string & k) {
    auto t = stack->get(idx);
    auto v = stack->pop();
    __setTable(t, luaValue(k), v);
}

void luaState::SetI(int idx, int64_t i) {
    auto t = stack->get(idx);
    auto v = stack->pop();
    __setTable(t, luaValue(i), v);
}

void luaState::pushLuaStack(luaStack * st){
    st->prev = this->stack;
    this->stack = st;
}

void luaState::popLuaStack() {
    auto st = this->stack;
    this->stack = st->prev;
    st->prev = nullptr;
}

void luaState::pushExFunction(ExFunction f){
    this->stack->emplace(new luaClosure(std::move(f), 0));
}

void luaState::pushExClosure(ExFunction f, int n) {
    auto closure = new luaClosure(std::move(f), n);
    for(auto i=n;i>0;i--){
        auto val = this->stack->pop();
        closure->upvals->at(n - 1) = &val;
    }
    this->stack->emplace(closure);
}

bool luaState::isExFunction(int idx){
    auto val = this->stack->get(idx);
    if (val.type() == luaValue::Closure){
        auto * c = val.get<luaValue::Closure>();
        return c->func != nullptr;
    }
    return false;
}

ExFunction luaState::ToExFunction(int idx){
    auto val = this->stack->get(idx);
    if (val.type() == luaValue::Closure){
        auto * c = val.get<luaValue::Closure>();
        return c->func;
    }
    return nullptr;
}


void luaState::PushGlobalTable(){
    auto global = this->registry->get(luaValue(LUA_RIDX_GLOBALS));
    this->stack->push(global);
}

LuaType luaState::GetGlobal(const std::string & name){
    auto t = this->registry->get(luaValue(LUA_RIDX_GLOBALS));
    return this->__getTable(t, luaValue(name));
}

void luaState::SetGlobal(const std::string & name){
    auto & t = this->registry->get(luaValue(LUA_RIDX_GLOBALS));
    auto v = this->stack->pop();
    this->__setTable(t, luaValue(name), v);
}


void luaState::Register(const std::string & name, ExFunction f){
    this->pushExFunction(std::move(f));
    this->SetGlobal(name);
}



