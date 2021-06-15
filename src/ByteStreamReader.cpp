//
// Created by ziyue on 2021/6/13.
//

#include "ByteStreamReader.h"
#include "byte.h"
#include "luaValue.h"
#include "instruction.h"
#include "opcodes.h"
#include <string>
#include <fstream>
#include <cstring>
#include <iostream>
#include <iomanip>

using namespace std::string_literals;

byte * readByteStream(const std::string & filename, long long & gcount, long long stepSize=1024){
    std::ifstream file;
    file.open(filename, std::ios::in | std::ios::binary);

    gcount = 0;
    byte * data = (byte*) malloc(stepSize);
    byte * newdata = nullptr;
    memset(data, 0, stepSize);

    int looptime = 1;
    while(true){
        file.read(reinterpret_cast<char *>(data + gcount), sizeof(byte) * stepSize);
        if (file.gcount() < stepSize){
            gcount += file.gcount();
            break;
        }else{
            looptime *= 2;
            gcount += stepSize;
            newdata = (byte*) malloc(stepSize * looptime);
            memcpy(newdata, data, gcount);
            data = newdata;
        }
    }
    return data;
}

byte * readByteStream(const std::string & filename){
    long long gcount = 0;
    return readByteStream(filename, gcount);
}

void printByteStream(byte * data, long long gcount, int interval = 16){
    long long count = 0;
    while(count < gcount){
        auto i = 0;
        for(i = 0; i < interval && count < gcount;i++, count++){
            std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                      << static_cast<int>(data[count]) << " ";
        }
        std::cout << "   ";
        auto k = i;
        while(k < interval){
            std::cout << "   ";
            k++;
        }
        for(auto j=0; j < i; j++){
            auto c = static_cast<char>(data[j+count -i]);
            if ( c > 32 && c < 125){
                std::cout << c;
            }else{
                std::cout << '.';
            }
        }
        std::cout << std::endl;
    }
}

reader::reader(byte* D):data{D}{};

reader::~reader()= default;

byte reader::readByte() {
    auto b = data[0];
    data++;
    return b;
}

uint32_t reader::readUint32() {
    byte* tn = new byte[4];
    memcpy(tn, data, 4);
    data+=4;
    auto* rn = (uint32_t*)(tn);
    return *rn;
}

uint64_t reader::readUint64() {
    byte* tn = new byte[8];
    memcpy(tn, data, 8);
    data+=8;
    auto* rn = (uint64_t*)(tn);
    return *rn;
}

int64_t reader::readLuaInteger() {
    auto rn = readUint64();
    auto irn = static_cast<int64_t>(rn);
    return irn;
}

double reader::readLuaNumber() {
    byte* tn = new byte[8];
    for(int i =0;i<8;i++){
        tn[i] = *data;
        data++;
    }
    auto *rn = (double*)(tn);
    return *rn;
}

std::string reader::readString() {
    auto size = static_cast<size_t>(readByte());

    if (size == 0){
        return ""s;
    } else if (size == 0xFF){
        size = static_cast<size_t>(readUint64());
    }
    auto bytes = readBytes(size - 1);
    auto str = new char[size];
    memcpy(str, bytes, size-1);
    str[size-1] = '\0';
    return std::string{str};
}

byte * reader::readBytes(size_t n) {
    byte* bytes = new byte[n];
    for(size_t i=0;i<n;i++){
        bytes[i] = data[i];
    }
    data+=n;
    return bytes;
}

bool reader::checkSignature() {
    bool ok = (data[0] == 0x1b && data[1] == 0x4c && data[2] == 0x75 && data[3] == 0x61);
    data+=4;
    return ok;
}

bool reader::checkLuaData() {
    bool ok = (data[0] == 0x19 && data[1] == 0x93
               && data[2] == 0x0D && data[3] == 0x0A
               && data[4] == 0x1A && data[5] == 0x0A);
    data+=6;
    return ok;
}

void reader::checkHeader() {
    if (!checkSignature()){
        std::cerr << "Not a precompiled chunk!" << std::endl;
        abort();
    }else if(readByte() != LUAC_VERSION){
        std::cerr << "Version mismatch!"<< std::endl;
        abort();
    }else if(readByte() != LUAC_FORMAT){
        std::cerr << "Format mismatch" << std::endl;
        abort();
    }else if(!checkLuaData()){
        std::cerr << "Corrupted!" << std::endl;
        abort();
    }else if(readByte() != CINT_SIZE){
        std::cerr << "int size mismatch" << std::endl;
        abort();
    }else if(readByte() != CSZIET_SIZE){
        std::cerr << "size_t size mismatch" << std::endl;
        abort();
    }else if(readByte() != INSTRUCTION_SIZE){
        std::cerr << "instruction size mismatch" << std::endl;
        abort();
    }else if (readByte() !=LUA_INTEGER_SIZE){
        std::cerr << "lua_Integer size mismatch" << std::endl;
        abort();
    }else if(readByte() != LUA_NUMBER_SIZE){
        std::cerr << "lua_Number size mismatch" << std::endl;
        abort();
    }else if(readLuaInteger() != LUAC_INT){
        std::cerr << "endianness mismatch!" << std::endl;
        abort();
    }else if(readLuaNumber() != LUAC_NUM){
        std::cerr << "float format mismatch" << std::endl;
        abort();
    }
}

std::vector<uint32_t> * reader::readCode() {
    auto code = new std::vector<uint32_t>(readUint32());
    for(unsigned int & i : *code){
        i = readUint32();
    }
    return code;
}

luaValue reader::readConstant() {
    int tag = static_cast<int>(readByte());
    if (tag == TAG_NIL){
        return luaValue(nil);
    }else if(tag == TAG_BOOLEN){
        return luaValue(readByte()!=0);
    }else if(tag == TAG_INTEGER){
        return luaValue(readLuaInteger());
    }else if(tag == TAG_NUMBER){
        return luaValue(readLuaNumber());
    }else if(tag == TAG_SHORT_STR || tag == TAG_LONG_STR){
        return luaValue(readString());
    }else{
        std::cerr << "Corrupted, Position: readConstant" << std::endl;
        abort();
    }
}

std::vector<Upvalue> * reader::readUpvalues() {
    auto upvalues = new std::vector<Upvalue>(readUint32());
    for(auto & upvalue : *upvalues){
        upvalue.Instack = readByte();
        upvalue.Idx = readByte();
    }
    return upvalues;
}



std::vector<luaValue> * reader::readConstants() {
    auto constants = new std::vector<luaValue>(readUint32());
    for(auto & constant : *constants){
        constant = readConstant();
    }
    return constants;
}

std::vector<Prototype *> * reader::readProtos(const std::string& parentSource) {
    auto protos = new std::vector<Prototype*>(readUint32());
    for(auto &proto: *protos){
        proto = readProto(parentSource);
    }
    return protos;
}

std::vector<uint32_t> * reader::readLineInfo() {
    auto lineInfo = new std::vector<uint32_t>(readUint32());
    for(auto &info: *lineInfo){
        info = readUint32();
    }
    return lineInfo;
}

std::vector<LocVar> * reader::readLocVars() {
    auto locVars = new std::vector<LocVar>(readUint32());
    for(auto & locvar: *locVars){
        locvar.VarName = readString();
        locvar.StartPC = readUint32();
        locvar.EndPC   = readUint32();
    }
    return locVars;
}

std::vector<std::string> * reader::readUpvalueNames() {
    auto names = new std::vector<std::string>(readUint32());
    for(auto & name : *names){
        name = readString();
    }
    return names;
}

Prototype* reader::readProto(std::string parentSource) {
    std::string source = readString();
    if (source.empty()){
        source = std::move(parentSource);
    }
    auto* rep = new Prototype();
    rep->Source = source;
    rep->LineDefined = readUint32();
    rep->LastLineDefined = readUint32();
    rep->NumParams = readByte();
    rep->IsVararg = readByte();
    rep->MaxStackSize = readByte();
    rep->Code = readCode();
    rep->Constants = readConstants();
    rep->Upvalues = readUpvalues();
    rep->Protos = readProtos(source);
    rep->LineInfo = readLineInfo();
    rep->LocVars = readLocVars();
    rep->UpvalueNames = readUpvalueNames();
    return rep;
}

void printHeader(Prototype* f){
    auto funcType{"main"s};
    if(f->LineDefined > 0){
        funcType = "function"s;
    }
    auto varargFlag{""s};
    if (f->IsVararg>0){
        varargFlag = "+"s;
    }
    std::cout << std::endl;
    std::cout << funcType << " <"
              << f->Source << ':'
              << f->LineDefined << ','
              << f->LastLineDefined << "> ("
              << f->Code->size() << ""
              << "instructions)" << std::endl;
    std::cout << f->NumParams << varargFlag
              << " params, " << (int)f->MaxStackSize
              << " slots" << f->Upvalues->size()
              << " upvalues, ";
    std::cout << f->LocVars->size() << " locals, "
              << f->Constants->size() << " constants, "
              << f->Protos->size() << " functions"
              << std::endl;
}

void printOperands(Instruction & i){
    auto mode = i.OpMode();

    switch (mode) {
        case IABC:
        {
            auto [a, b, c] = i.ABC(); std::cout<< a << " ";
            if (i.BMode() != OpArgN){
                if (b > 0xFF){ std::cout << -1- (b & 0xFF) << " ";}
                else { std::cout << b << " ";}
            }
            if (i.CMode() != OpArgN){
                if (c > 0xFF){std::cout << -1- (c & 0xFF) << " "; }
                else{ std::cout << c << " ";}
            }
        }
            break;
        case IABx:
        {
            auto [a, bx] = i.ABx(); std::cout<< a << " ";
            if (i.BMode() == OpArgK){std::cout << -1-bx << " ";}
            else if (i.BMode() == OpArgK){std::cout << bx << " ";}
        }
            break;
        case IAsBx:
        {
            auto [a, sbx] = i.ABx(); std::cout<< a << " " << sbx << " ";
        }
            break;
        case IAx:
        {
            auto a = i.Ax(); std::cout<< a << " ";
        }
    }
}

void printCode(Prototype* f){
    for(size_t pc=0; pc < f->Code->size() ;pc++){
        auto line{"-"s};
        auto &c{f->Code->at(pc)};
        if(!f->LineInfo->empty()){
            line = std::to_string(f->LineInfo->at(pc));
        }
        auto i{Instruction{c}};
        std::cout << "\t" << pc+1 << "\t[" << line << "]\t" << i.OpName() << " \t";
        printOperands(i);
        std::cout << std::endl;
    }
}

std::string constantToString(luaValue & k){
    switch (k.index()) {
        case luaValue::Nil:
            return "nil";
        case luaValue::Int:
            return std::to_string(k.get<int64_t>());
        case luaValue::Float:
            return std::to_string(k.get<double>());
        case luaValue::Boolean:
            if (k.get<bool>()){
                return "true";
            }else{
                return "false";
            }
        case luaValue::String:
            return k.get<std::string>();
        default:
            return "?";

    }
}

std::string upvalName(Prototype * f, int idx){
    if (!f->UpvalueNames->empty()){
        return f->UpvalueNames->at(idx);
    }
    return "-";
}

void printDetail(Prototype* f){
    std::cout << "constants (" << f->Constants->size() << "):"<< std::endl;
    for(size_t i=0;i < f->Constants->size();i++){
        std::cout << "\t" << i+1 << "\t" << constantToString(f->Constants->at(i)) <<  std::endl;
    }
    std::cout << "locals (" << f->LocVars->size() << "):" << std::endl;
    for(size_t i=0;i<f->LocVars->size();i++){
        auto &locvar = f->LocVars->at(i);
        std::cout << "\t" << i << "\t"
                  << locvar.VarName   << "\t"
                  << locvar.StartPC+1 << "\t"
                  << locvar.EndPC+1   << std::endl;
    }
    std::cout << "Upvalues (" << f->Upvalues->size()<< "):" << std::endl;
    for(size_t i = 0;i < f->Upvalues->size();i++){
        auto &upval = f->Upvalues->at(i);
        std::cout << "\t" << i << "\t"
                  << upvalName(f, i) << "\t"
                  << (int)upval.Instack   << "\t"
                  << (int)upval.Idx       << std::endl;
    }
}

void byteStreamSimpleAnalysis(Prototype* f){
    printHeader(f);
    printCode(f);
    printDetail(f);
    for(auto & p : *f->Protos){
        byteStreamSimpleAnalysis(p);
    }
}