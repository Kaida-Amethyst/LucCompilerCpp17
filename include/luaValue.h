//
// Created by ziyue on 2021/6/13.
//

#ifndef LUACOMPILER_RENEW_LUAVALUE_H
#define LUACOMPILER_RENEW_LUAVALUE_H

#include <variant>
#include "nil.h"
#include <cstdint>
#include <string>
#include "consts.h"
#include <iostream>
#include "luaClosure.h"


class luaTable;

extern luaValue lnil;

class luaValue{
private:
    std::variant<Nil, int64_t, double, bool, std::string, luaTable*, luaClosure*> value;
public:
    static const size_t Nil     = 0;
    static const size_t Int     = 1;
    static const size_t Float   = 2;
    static const size_t Boolean = 3;
    static const size_t String  = 4;
    static const size_t Table   = 5;
    static const size_t Closure = 6;

    luaValue():value(nil){}
    explicit luaValue(int n):value(static_cast<int64_t>(n)){}
    explicit luaValue(class Nil n):value(nil){}
    explicit luaValue(int64_t n):value(n){}
    explicit luaValue(double n):value(n){}
    explicit luaValue(bool n):value(n){}
    explicit luaValue(std::string n):value(n){}
    explicit luaValue(std::nullptr_t n):value(nil){}
    explicit luaValue(luaTable * t):value(t){}
    explicit luaValue(luaClosure * c):value(c){}
    explicit luaValue(Prototype * p):value(new luaClosure(p)){}
//    explicit luaValue(const char n):value(nil){};
    luaValue(luaValue & l)  = default;
    luaValue(const luaValue & l) = default;
    luaValue(luaValue && l) = default;

    luaValue& operator=(const luaValue & v) = default;
    luaValue& operator=(const ::Nil & n);
    luaValue& operator=(const int     &i);
    luaValue& operator=(const int64_t &i);
    luaValue& operator=(const double  &d);
    luaValue& operator=(const bool    &b);
    luaValue& operator=(const std::string & s);

    inline bool isNil() const noexcept{
        return value.index() == luaValue::Nil;
    }

    size_t index() const noexcept;
    size_t type() const noexcept;

    template<typename T>
    T & get(){
        return std::get<T>(this->value);
    }

    template<size_t idx>
    auto & get() const {
        return std::get<idx>(this->value);
    }

    friend LuaType typeOf(luaValue &val);
    friend std::hash<luaValue>;
};

LuaType typeOf(luaValue &val);

namespace std{
    template <>
    struct hash<luaValue>{
        using argument_type = luaValue;
        using result_type = size_t ;
        result_type operator()(const argument_type & c) const{
            switch (c.type()) {
                case luaValue::Nil:
                    return 0;
                case luaValue::Int:
                    return std::get<luaValue::Int>(c.value);
                case luaValue::Float:
                    return std::hash<double>{}(std::get<luaValue::Float>(c.value));
                case luaValue::Boolean:
                    return std::get<bool>(c.value)?1:0;
                case luaValue::String:
                    return std::hash<std::string>{}(std::get<luaValue::String>(c.value));
                case luaValue::Table:
                    std::cerr << "Cannot be a Table!" << std::endl;
                    abort();
                case luaValue::Closure:
                    std::cerr << "Cannot be a closure!" << std::endl;
                    abort();
                default:
                    std::cerr << "Unknown luaValue Type!" << std::endl;
                    abort();
            }
        }
    };
}

bool operator==(const luaValue & l, const luaValue & r);
bool operator==(const luaValue & l, const Nil &);
bool operator==(const luaValue & l, const int & n);
bool operator==(const luaValue & l, const int64_t & n);
bool operator==(const luaValue & l, const double & d);
bool operator==(const luaValue & l, const bool & b);
bool operator==(const luaValue & l, const std::string & str);
bool operator==(const luaValue & l, const luaTable & t);
bool operator==(const luaTable & l, const luaTable & r);

bool operator==(const Nil &,  const luaValue & l);
bool operator==(const int & n,           const luaValue & l);
bool operator==(const int64_t & n,       const luaValue & l);
bool operator==(const double & d,        const luaValue & l );
bool operator==(const bool & b,          const luaValue & l);
bool operator==(const std::string & str, const luaValue & l);
bool operator==(const luaTable & t,      const luaValue & l);

std::ostream& operator<<(std::ostream & os, luaValue & v);

#endif //LUACOMPILER_RENEW_LUAVALUE_H
