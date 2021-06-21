#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <memory>
#include "lexer.h"
#include "token.h"
#include <iomanip>
#include "parser.h"
#include "chunkStruct.h"
#include "codeGen.h"
#include "luaVM.h"
#include "ByteStreamReader.h"

int print(LuaVM * ls){
    auto nArgs = ls->GetTop();
    for(auto i=1;i<=nArgs;i++){
        if (ls->IsBoolean(i)){
            std::cout << std::boolalpha << ls->ToBoolean(i);
        }else if (ls->IsString(i)){
            std::cout << ls->ToString(i);
        }else{
            std::cout << luaState::TypeName(ls->Type(i)) << std::endl;
        }
        if (i < nArgs){
            std::cout << "\t";
        }
    }
    std::cout << std::endl;
    return 0;
}

int getmetatable(LuaVM * vm){
    if (!vm->GetMetatable(1)){
        vm->PushNil();
    }
    return 1;
}

int setmetatable(LuaVM * vm){
    vm->SetMetatable(1);
    return 1;
}

int next(LuaVM * vm){
    vm->SetTop(2);
    if (vm->Next(1)){
        return 2;
    }else{
        vm->PushNil();
        return 1;
    }
}

int pairs(LuaVM * vm){
    vm->pushExFunction(next);
    vm->PushValue(1);
    vm->PushNil();
    return 3;
}

int _iPairsAux(LuaVM * vm){
    auto i = vm->ToInteger(2) + 1;
    vm->PushInteger(i);
    if (vm->GetI(1, i) == LUA_TNIL){
        return 1;
    }else{
        return 2;
    }
}

int iPairs(LuaVM * vm){
    vm->pushExFunction(_iPairsAux);
    vm->PushValue(1);
    vm->PushInteger(0);
    return 3;
}






int main() {
    std::ifstream file;
    file.open("../luafiles/hello_world.lua", std::ios::in);
    std::istreambuf_iterator<char> beg(file), end;
    std::string st{beg, end};

    auto ls = new LuaVM();
    ls->Register("print", print);
    ls->Register("next", next);
    ls->Register("pairs", pairs);
    ls->Register("ipairs", iPairs);

    ls->Load(st, "test");
    ls->Call(0, 0);

//    Lexer lexer{"codes", st};
//    parseBlock(lexer);
    return 0;
}
