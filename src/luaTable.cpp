//
// Created by ziyue on 2021/6/14.
//

#include "luaTable.h"
#include "luaMath.h"
#include <iostream>

luaTable::luaTable(int nArr, int nRec) {
    arr = nArr > 0? new std::vector<luaValue>(nArr):new std::vector<luaValue>();
    _map = new std::unordered_map<luaValue, luaValue>();
}

luaTable::luaTable(const luaTable & other){
    arr = new std::vector<luaValue>(*(other.arr));
    _map = new std::unordered_map<luaValue, luaValue>(*(other._map));
}

luaTable::luaTable(luaTable && other){
//    arr.reset(); _map.reset();
    delete arr; delete _map;
    arr = new std::vector<luaValue>(*other.arr);
    _map = new std::unordered_map<luaValue, luaValue>(*other._map);
}

luaTable& luaTable::operator=(const luaTable & other){
//    arr.reset(); _map.reset();
    delete arr; delete _map;
    arr = new std::vector<luaValue>(*other.arr);
    _map = new std::unordered_map<luaValue, luaValue>(*other._map);
    return *this;
}

luaTable::~luaTable() {
    delete arr; delete _map;
}

bool operator==(const luaTable & l, const luaTable & r){
    return &l == &r;
}


luaValue & luaTable::get(const luaValue & key) const {
    if (key.type() == luaValue::Int){
        auto idx = key.get<luaValue::Int>();
        if (idx >= 1 && idx <= arr->size()) return arr->at(idx-1);
    }else if (key.type() == luaValue::Float){
        if (auto [idx, ok] = FloatToInteger(key.get<luaValue::Float>()); ok){
            if (idx >= 1 && idx <= arr->size()) return arr->at(idx-1);
        }
    }
    return _map->at(key);

}

luaValue luaTable::__floatToInteger(const luaValue &key) const {
    if (key.type() == luaValue::Float){
        auto num = key.get<luaValue::Float>();
        if (auto [i, ok] = FloatToInteger(num); ok){
            return luaValue(i);
        }
    }
    return luaValue(key);
}

void luaTable::__shrinkArray(){
    auto iter = arr->end()-1;
    while(iter->isNil()){
        iter --;
    }
    iter++;
    if (iter != arr->end()){
        arr->erase(iter, arr->end());
    }
}

void luaTable::__expandArray() {
    luaValue K;
    for(int idx = arr->size()+1;true;idx++){
        K = idx;
        if (_map->find(K) != _map->end()){
            _map->erase(K);
            arr->push_back(_map->at(K));
        }else{
            break;
        }
    }
}


void luaTable::put(luaValue K, luaValue V) {
    switch (K.type()) {
        case luaValue::Nil:
            std::cerr << "Trying to make index equal to Nil!" << std::endl;
            abort();
        case luaValue::Float:
            if (std::isnan(K.get<luaValue::Float>())){
                std::cerr << "Trying to insert an index whihc is NaN!" << std::endl;
                abort();
            }
            if (auto[i, ok] = FloatToInteger(K.get<luaValue::Float>()); ok){
                K = i;
            }
        case luaValue::Int:
            if (K.type() == luaValue::Int){
                if (auto idx =  K.get<luaValue::Int>(); idx>= 1){
                    if (idx <= arr->size()){
                        arr->at(idx-1) = V;
                        // mark!
                        if (idx == arr->size() && V == nil){
                            __shrinkArray();
                        }
                        return ;
                    }else if (idx == arr->size()+1){
                        _map->erase(K);
                        if (V.type() != luaValue::Nil){
                            arr->push_back(V);
                            __expandArray();
                            return ;
                        }
                    }
                }
            }
        default:
            if (!V.isNil()){
                if (_map->find(K) == _map->end()){
                    _map->insert(std::pair{K, V});
                }else{
                    _map->at(K) = V;
                }
            }else{
                _map->erase(K);
            }
    }
}
