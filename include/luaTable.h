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
    std::unordered_map<luaValue, luaValue> * keys;
    bool changed;
    luaTable * metatable;
    luaValue __floatToInteger(const luaValue & key) const ;
    void __shrinkArray();
    void __expandArray();
    void initKeys();
public:
    luaTable(int nArr, int nRec);
    luaTable(const luaTable & other);
    luaTable(luaTable && other);
    ~luaTable();
    luaValue & get(const luaValue & key) const ;
    inline bool find(const luaValue & key) const{
        return _map->find(key) != _map->end();
    }
    inline bool hasMetafield(const std::string & fieldName) const{
        return this->metatable != nullptr && !(this->metatable->get(luaValue(fieldName)) == nil);
    }
    void put(luaValue K, luaValue V);
    [[nodiscard]] size_t len() const noexcept{
        return _map->size();
    }

    luaValue nextKey(const luaValue & key);

    luaTable& operator=(const luaTable & other);

    friend bool operator==(const luaTable & l, const luaTable & r);
    friend bool operator==(const luaValue & l, const luaValue & r);
    friend class luaValue;
    friend class luaState;
    friend class LuaVM;
    friend class luaStack;
};



#endif //LUACOMPILER_RENEW_LUATABLE_H
