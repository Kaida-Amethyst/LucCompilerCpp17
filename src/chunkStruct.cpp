//
// Created by ziyue on 2021/6/13.
//

#include "chunkStruct.h"
#include "ByteStreamReader.h"

int TAG_NIL = 0x00;
int TAG_BOOLEN = 0x01;
int TAG_NUMBER = 0x03;
int TAG_INTEGER = 0x13;
int TAG_SHORT_STR = 0x04;
int TAG_LONG_STR = 0x14;



Prototype* Undump(byte* data){
    auto R = reader(data);
    R.checkHeader();
    R.readByte();
    return R.readProto("");
}