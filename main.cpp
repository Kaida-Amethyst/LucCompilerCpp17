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


int main() {
//    long long gcount = 0;
//    auto * data = readByteStream("../luafiles/testTable.luac", gcount, 1024);
//    printByteStream(data, gcount, 16);

    auto * data = readByteStream("../luafiles/testClosure.luac");
    auto p = Undump(data);

    auto ls = new LuaVM();

    byteStreamSimpleAnalysis(p);

    ls->Load((byte*)data, "test", 'b');
    ls->Call(0, 0);

    return 0;
}
