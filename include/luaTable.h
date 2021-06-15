//
// Created by ziyue on 2021/6/14.
//

#ifndef LUACOMPILER_RENEW_LUATABLE_H
#define LUACOMPILER_RENEW_LUATABLE_H

#include <memory>
#include <vector>
#include <unordered_map>
#include "luaValue.h"


class luaTable{
private:
    std::vector<luaValue> * arr;
    std::unordered_map<luaValue, luaValue> * _map;
    luaValue __floatToInteger(const luaValue & key) const ;
    void __shrinkArray();
    void __expandArray();
public:
    luaTable(int nArr, int nRec);
    luaTable(const luaTable & other);
    luaTable(luaTable && other);
    ~luaTable();
    luaValue & get(const luaValue & key) const ;
    void put(luaValue K, luaValue V);
    [[nodiscard]] size_t len() const noexcept{
        return _map->size();
    }
    luaTable& operator=(const luaTable & other);

    friend bool operator==(const luaTable & l, const luaTable & r);
    friend class luaState;
    friend class LuaVM;
    friend class luaStack;
};



#endif //LUACOMPILER_RENEW_LUATABLE_H
