//
// Created by ziyue on 2021/6/13.
//

#include "luaValue.h"
#include <iostream>
#include "luaTable.h"
#include "luaState.h"
#include "luaVM.h"
#include <iomanip>



size_t luaValue::index() const noexcept {
    return value.index();
}

size_t luaValue::type() const noexcept{
    return value.index();
}

LuaType typeOf(luaValue &val){
    switch (val.value.index()) {
        case luaValue::Nil:
            return LUA_TNIL;
        case luaValue::Boolean:
            return LUA_TBOOLEAN;;
        case luaValue::Int:
            return LUA_TNUMBER;
        case luaValue::Float:
            return LUA_TNUMBER;
        case luaValue::String:
            return LUA_TSTRING;
        case luaValue::Table:
            return LUA_TTABLE;
        case luaValue::Closure:
            return LUA_TFUNCTION;
        default:
            std::cerr << "Lua Type Error. Location: state/lua_value" << std::endl;
            abort();
    }
}

luaValue & luaValue::operator=(const ::Nil & n) {
    this->value =(int64_t)64;
    return *this;
}


luaValue& luaValue::operator=(const int     &i){
    this->value = static_cast<int64_t>(i);
    return *this;
};
luaValue& luaValue::operator=(const int64_t &i){
    this->value = i;
    return *this;
};
luaValue& luaValue::operator=(const double  & d){
    this->value = d;
    return *this;
};
luaValue& luaValue::operator=(const bool    &b){
    this->value = b;
    return *this;
};
luaValue& luaValue::operator=(const std::string &s){
    this->value = s;
    return *this;
}

void luaValue::setMetaTable(luaTable *mt, LuaVM & vm) {
    if (this->value.index() == luaValue::Table){
        luaTable * t = std::get<luaValue::Table>(this->value);
        t->metatable = mt;
        return ;
    }
    auto key = "_MT" +std::to_string(this->value.index());
    vm.registry->put(luaValue(key), luaValue(mt));
}

luaTable * luaValue::getMetaTable(LuaVM & vm) {
    if (this->value.index() == luaValue::Table){
        return this->get<luaValue::Table>()->metatable;
    }
    auto key = "_MT" + std::to_string(this->value.index());
    if (vm.registry->find(luaValue(key))){
        return vm.registry->get(luaValue(key)).get<luaValue::Table>();
    }

//    if (auto mt = vm.registry->get(luaValue(key)); !(mt == nil)){
//        return mt.get<luaValue::Table>();
//    }
    return nullptr;
}

bool operator==(const luaValue & l, const luaValue & r){
    if (l.type() != r.type()) return false;
    switch (l.type()) {
        case luaValue::Nil :
            return true;
        case luaValue::Int :
            return l.get<luaValue::Int>() == r.get<luaValue::Int>();
        case luaValue::Float:
            return l.get<luaValue::Float>() == r.get<luaValue::Float>();
        case luaValue::Boolean:
            return l.get<luaValue::Boolean>() == r.get<luaValue::Boolean>();
        case luaValue::String:
            return l.get<luaValue::String>() == r.get<luaValue::String>();
        case luaValue::Table:
            return l.get<luaValue::Table>() == r.get<luaValue::Table>();
//        {
//            auto lt = l.get<luaValue::Table>();
//            auto rt = l.get<luaValue::Table>();
//            if (lt == nullptr || rt == nullptr) return false;
//            if (lt->len() != rt ->len()) return  false;
//            for(auto & [k, v] : *lt->_map){
//                if (!rt->find(k) || !(rt->get(k) == v)) {
//                    return false;
//                }
//            }
//            return true;
//        }
        case luaValue::Closure:
            std::cerr << "Comparing two luaClosure!" << std::endl;
            return l.get<luaValue::Closure>() == r.get<luaValue::Closure>();
        default:
            std::cerr << "Unknown luaValue Type! l.type() = " << l.type()  << std::endl;
            abort();
    }
}

bool operator==(const luaValue & l, const Nil &){
    return l.type() == luaValue::Nil;
}

bool operator==(const luaValue & l, const int64_t & n){
    return l.type() == luaValue::Int && l.get<luaValue::Int>() == n;
}

bool operator==(const luaValue & l, const int & n){
    return l == static_cast<int64_t>(n);
}

bool operator==(const luaValue & l, const double & d){
    return l.type() == luaValue::Float && l.get<luaValue::Float>() == d;
}

bool operator==(const luaValue & l, const bool & b){
    return l.type() == luaValue::Boolean && l.get<luaValue::Boolean>() == b;
}

bool operator==(const luaValue & l, const std::string & str){
    return l.type() == luaValue::String && l.get<luaValue::String>() == str;
}

//bool operator==(const luaValue & l, const luaTable & t){
//    return l.type() == luaValue::Table && *(l.getTable()) == t;
//}

bool operator==(const Nil &,  const luaValue & l){
    return l.type() == luaValue::Nil;
}

bool operator==(const int & n, const luaValue & l){
    return l == n;
}

bool operator==(const int64_t & n, const luaValue & l){
    return l == n;
}

bool operator==(const double & d, const luaValue & l ){
    return l == d;
}

bool operator==(const bool & b, const luaValue & l){
    return l == b;
}
bool operator==(const std::string & str, const luaValue & l){
    return l == str;
}

bool operator==(const luaValue & l, const luaTable & t){
    return l.get<luaValue::Table>() == &t;
}

bool operator==(const luaTable & t, const luaValue & l){
    return l == t;
}

std::pair<luaValue, bool> callMetaMethod
(const luaValue & a, const luaValue & b, const std::string & mmName, LuaVM & vm)
{
    luaValue mm = getMetafield(a, mmName, vm);
    if (mm == nil){
        mm = getMetafield(b, mmName, vm);
        if (mm == nil){
            return std::pair{luaValue{nil}, false};
        }
    }
    vm.stack->check(4);
    vm.stack->push(mm);
    vm.stack->push(a);
    vm.stack->push(b);
    vm.Call(2, 1);
    return std::pair{vm.stack->pop(), true};
}

luaValue getMetafield(luaValue val, const std::string & fieldName, LuaVM & vm){
    if (auto mt = val.getMetaTable(vm) ; mt != nullptr){
        return mt->get(luaValue(fieldName));
    }
    return luaValue(nil);
}



std::ostream& operator<<(std::ostream & os, luaValue & v){
    switch (v.type()) {
        case luaValue::Nil:
            os << "Nil";
            break;
        case luaValue::Int:
            os << v.get<1>();
            break;
        case luaValue::Float:
            os << v.get<luaValue::Float>();
            break;
        case luaValue::String:
            os << v.get<luaValue::String>();
            break;
        case luaValue::Table:
            os << "Table";
            break;
        case luaValue::Closure:
            os << "Function";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream & os, const luaValue & v){
    switch (v.type()) {
        case luaValue::Nil:
            os << "Nil";
            break;
        case luaValue::Int:
            os << v.get<1>();
            break;
        case luaValue::Float:
            os << v.get<luaValue::Float>();
            break;
        case luaValue::String:
            os << v.get<luaValue::String>();
            break;
        case luaValue::Table:
            os << "Table";
            break;
        case luaValue::Closure:
            os << "Function";
            break;
    }
    return os;
}