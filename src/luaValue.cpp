//
// Created by ziyue on 2021/6/13.
//

#include "luaValue.h"
#include <iostream>


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
//        case luaValue::Table:
//            return l.getTable() == r.getTable();
        default:
            std::cerr << "Unknown luaValue Type!" << std::endl;
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