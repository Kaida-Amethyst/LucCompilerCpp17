#include <iostream>
#include <variant>
#include <iomanip>
#include "luaValue.h"
#include <vector>
#include "luaState.h"
#include "luaVM.h"
#include "instruction.h"
#include "opcodes.h"
#include "ByteStreamReader.h"
#include "luaTable.h"

using namespace std::string_literals;

int print(luaState * ls){
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


int main() {
//    long long gcount = 0;
//    auto * data = readByteStream("../luafiles/testTable.luac", gcount, 1024);
//    printByteStream(data, gcount, 16);

    auto * data = readByteStream("../luafiles/ch10.luac");


    auto ls = new LuaVM();
    ls->Register("print", print);


    ls->Load((byte*)data, "test", 'b');
    ls->Call(0, 0);

    return 0;
}
