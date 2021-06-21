//
// Created by ziyue on 2021/6/17.
//

#include "funcInfo.h"
#include "opcodes.h"
#include "token.h"
#include "chunkStruct.h"
#include "instruction.h"
#include <iostream>
#include <map>


static std::map<int, int> arithAndBitwiseBinops {
        {TOKEN_OP_ADD,  OP_ADD,},
        {TOKEN_OP_SUB,  OP_SUB},
        {TOKEN_OP_MUL,  OP_MUL},
        {TOKEN_OP_MOD,  OP_MOD},
        {TOKEN_OP_POW,  OP_POW},
        {TOKEN_OP_DIV,  OP_DIV},
        {TOKEN_OP_IDIV, OP_IDIV},
        {TOKEN_OP_BAND, OP_BAND},
        {TOKEN_OP_BOR,  OP_BOR},
        {TOKEN_OP_BXOR, OP_BXOR},
        {TOKEN_OP_SHL,  OP_SHL},
        {TOKEN_OP_SHR,  OP_SHR},
};
        
//arithAndBitwiseBinops = map[int]int{
//TOKEN_OP_ADD:  OP_ADD,
//TOKEN_OP_SUB:  OP_SUB,
//TOKEN_OP_MUL:  OP_MUL,
//TOKEN_OP_MOD:  OP_MOD,
//TOKEN_OP_POW:  OP_POW,
//TOKEN_OP_DIV:  OP_DIV,
//TOKEN_OP_IDIV: OP_IDIV,
//TOKEN_OP_BAND: OP_BAND,
//TOKEN_OP_BOR:  OP_BOR,
//TOKEN_OP_BXOR: OP_BXOR,
//TOKEN_OP_SHL:  OP_SHL,
//TOKEN_OP_SHR:  OP_SHR,
//}


static int Int2fb(int x){
    auto e = 0;
    if (x < 8) return x;
    while(x >= (8 << 4)){
        x = (x+0xf) >> 4;
        e+=4;
    }
    while(x >= (8 << 1)){
        x = (x+1) >> 1;
        e++;
    }
    return ((e+1) << 3) | (x-8);
}

locVarInfo::locVarInfo(locVarInfo * p, const std::string & n, int scopelv, int sl, bool c):
prev(p), name(new std::string{n}), scopeLv(scopelv), slot(sl), captured(c)
{}

upvalInfo::upvalInfo(int ls, int ui, int ind) : locVarSlot{ls}, upvalIndex{ui}, index{ind} {}

//funcInfo::funcInfo()
//: constants (new std::unordered_map<luaValue, int>()),
//usedRegs(0), maxRegs(0),
//scopeLv(0), locVars(new std::vector<locVarInfo*>()),
//locNames(new std::unordered_map<std::string, locVarInfo*>()),
//breaks{new std::vector<std::vector<int> * >()},
//upvalues{new std::unordered_map<std::string, upvalInfo*>()},
//insts{new std::vector<uint32_t>()}
//{}

funcInfo::funcInfo(funcInfo *par, FuncDefExp *fd)
: parent(par), subFuncs(new std::vector<funcInfo *>()),
  constants (new std::unordered_map<luaValue, int>()),
  upvalues{new std::unordered_map<std::string, upvalInfo*>()},
  locNames(new std::unordered_map<std::string, locVarInfo*>()),
  locVars(new std::vector<locVarInfo*>()),
  breaks{new std::vector<std::vector<int> * >()},
  insts{new std::vector<uint32_t>()},
  isVararg{fd->IsVararg},
  numParams{fd->ParList == nullptr? 0:static_cast<int>(fd->ParList->size())},
  usedRegs(0), maxRegs(0), scopeLv(0)
{}

int funcInfo::indexOfConstant(const luaValue &k) {
    if (constants->find(k) != constants->end()){
        return constants->at(k);
    }
    auto idx = static_cast<int>(this->constants->size());
    constants->emplace(k, idx);
    return idx;
}

int funcInfo::allocReg() {
    this->usedRegs ++ ;
    if (this->usedRegs >= 255){
        std::cerr << "function or expression needs too many registers" << std::endl;
        abort();
    }
    if (this->usedRegs > this->maxRegs){
        this->maxRegs = this->usedRegs;
    }
    return this->usedRegs - 1;
}

int funcInfo::allocRegs(int n) {
    for (auto i = 0; i < n ;i++){
        this->allocReg();
    }
    return this->usedRegs - n;
}

void funcInfo::freeReg() {
    this->usedRegs -- ;
}

void funcInfo::freeRegs(int n) {
    while(n--){
        this->freeReg();
    }
}

void funcInfo::enterScope(bool breakable) {
    this->scopeLv ++ ;
    if (breakable){
        this->breaks->push_back(new std::vector<int>());
    }else{
        this->breaks->push_back(nullptr);
    }
}

int funcInfo::addLocVar(const std::string & name) {
    auto prev = this->locNames->find(name) != this->locNames->end() ? this->locNames->at(name) : nullptr;
    auto newVar = new locVarInfo{prev, name,  this->scopeLv, this->allocReg(), false};
    this->locVars->push_back(newVar);
    this->locNames->emplace(name, newVar);
    return newVar->slot;
}

int funcInfo::slotOfLocVar(const std::string &name) {
    if (this->locNames->find(name) != this->locNames->end()){
        this->locNames->at(name)->slot;
    }
    return -1;
}

void funcInfo::removeLocVar(locVarInfo *locVar) {
    this->freeReg();
    if (locVar->prev == nullptr){
        delete this->locNames;
        delete locVar->name;
    }else if (locVar->prev->scopeLv == locVar->scopeLv){
        this->removeLocVar(locVar->prev);
    }else{
        this->locNames->emplace(*locVar->name, locVar->prev);
    }
}

void funcInfo::exitScope() {
    if (this->breaks->empty()) return ;
    auto pendingBreakJmps = *(this->breaks->end() - 1);
    this->breaks->erase(this->breaks->end() - 1 );
    auto a = this->getJmpArgA();
    for(auto & p : * pendingBreakJmps){
        auto sBx = this->pc() - p;
        auto i = (sBx + MAXARG_sBx) << 14 | a << 6 | OP_JMP;
        try{
            this->insts->at(p) = static_cast<uint32_t>(i);
        }catch (...){
            std::cerr << "error! location:exitScope(), this->insts->size() = " <<  this->insts->size()
            << " while p = " << p << std::endl;
            abort();
        }

    }

    this->scopeLv -- ;
    for(auto [_, locVar] : *this->locNames){
        if (locVar->scopeLv > this->scopeLv){
            this->removeLocVar(locVar);
        }
    }
}

void funcInfo::addBreakJmp(int pc) {
    for(auto i = this->scopeLv; i>=0 ; i--){
        /*
         * For Debug, begin;
         * */
        if (i>=this->breaks->size()){
            std::cerr << "error! i>=this->breaks->size(), i = " << i
            << " this->breaks->size() = " << this->breaks->size() << std::endl;
            abort();
        }
        /*
         * For Debug, end;
         * */

        if (this->breaks->at(i) != nullptr){
            this->breaks->at(i)->push_back(pc);
            return ;
        }
    }
    std::cerr << "<break> at line ? not inside a loop! Location: funcInfo::addBreakJmp" << std::endl;
    abort();
}

int funcInfo::getJmpArgA() {
    auto hasCapturedLocVars = false;
    auto minSlotOfLocVars = this->maxRegs;
    for(auto & [_, locVar] : *this->locNames){
        if (locVar->scopeLv == this->scopeLv){
            for(auto & v = locVar; v != nullptr && v->scopeLv == this->scopeLv; v = v->prev){
                if (v->captured){
                    hasCapturedLocVars = true;
                }
                if (v->slot < minSlotOfLocVars && v->name->at(0) != '('){
                    minSlotOfLocVars = v->slot;
                }
            }
        }
    }
    if (hasCapturedLocVars){
        return minSlotOfLocVars + 1;
    }else {
        return 0;
    }
}

int funcInfo::pc() {
    return static_cast<int>(this->insts->size()) - 1;
}

void funcInfo::fixSbx(int pc, int sBx) {
    try{
        auto i = this->insts->at(pc);
        i = i << 18 >> 18;
        i = i | static_cast<uint32_t>(sBx + MAXARG_sBx) << 14;
        this->insts->at(pc) = i;
    }catch (...){
        std::cerr << "error, location: funcInfo::fixSbx, this->insts->size() = " <<  this->insts->size()
        << " pc = " << pc << std::endl;
        abort();
    }

}

int funcInfo::indexOfUpval(const std::string & name) {
    if (this->upvalues->find(name) != this->upvalues->end()){
        return this->upvalues->at(name)->index;
    }
    if (this->parent != nullptr){
        if (this->parent->locNames->find(name) != this->parent->locNames->end()){
            auto & locVar = this->parent->locNames->at(name);
            auto idx = static_cast<int>(this->upvalues->size());
            this->upvalues->emplace(name, new upvalInfo{locVar->slot, -1, idx});
            locVar->captured = true;
            return idx;
        }
        if (auto uvIdx = this->parent->indexOfUpval(name); uvIdx >= 0){
            auto idx = static_cast<int>(this->upvalues->size());
            this->upvalues->emplace(name, new upvalInfo{-1, uvIdx, idx});
            return idx;
        }
    }
    return -1;
}

void funcInfo::closeOpenUpvals() {
    auto a = this->getJmpArgA();
    if ( a > 0){
        this->emitJmp(a, 0);
    }
}

void funcInfo::emitABC(int opcode, int a, int b, int c){
    auto i = b << 23 | c<<14 | a << 6 | opcode;
    this->insts->push_back(static_cast<uint32_t>(i));
}

void funcInfo::emitABx(int opcode, int a, int bx){
    auto i = bx << 14 | a << 6 | opcode;
    this->insts->push_back(static_cast<uint32_t>(i));

}

void funcInfo::emitAsBx(int opcode, int a, int b ){
    auto i = (b + MAXARG_sBx) << 14 | a << 6 | opcode;
    this->insts->push_back(static_cast<uint32_t>(i));
}

void funcInfo::emitAx(int opcode, int ax){
    auto i = ax << 6 | opcode;
    this->insts->push_back(static_cast<uint32_t>(i));
}

void funcInfo::emitMove(int a, int b) {
    this->emitABC(OP_MOVE, a, b, 0);
}

// r[a], r[a+1], ..., r[a+b] = nil
void funcInfo::emitLoadNil(int a, int n) {
    this->emitABC(OP_LOADNIL, a, n-1, 0);
}

// r[a] = (bool)b; if (c) pc++
void funcInfo::emitLoadBool(int a, int b, int c) {
    this->emitABC(OP_LOADBOOL, a, b, c);
}

// r[a] = kst[bx]
void funcInfo::emitLoadK(int a, const luaValue & k) {
    auto idx = this->indexOfConstant(k);
    if (idx < (1 << 18)) {
        this->emitABx(OP_LOADK, a, idx);
    } else {
        this->emitABx(OP_LOADKX, a, 0);
        this->emitAx(OP_EXTRAARG, idx);
    }
}



// r[a], r[a+1], ..., r[a+b-2] = vararg
void funcInfo::emitVararg(int a, int n) {
    this->emitABC(OP_VARARG, a, n+1, 0);
}

// r[a] = emitClosure(proto[bx])
void funcInfo::emitClosure(int a, int bx) {
    this->emitABx(OP_CLOSURE, a, bx);
}

// r[a] = {}
void funcInfo::emitNewTable(int a, int nArr, int nRec) {
    this->emitABC(OP_NEWTABLE, a, Int2fb(nArr), Int2fb(nRec));
}

// r[a][(c-1)*FPF+i] := r[a+i], 1 <= i <= b
void funcInfo::emitSetList(int a, int b, int c) {
this->emitABC(OP_SETLIST, a, b, c);
}

// r[a] := r[b][rk(c)]
void funcInfo::emitGetTable(int a, int b, int c) {
    this->emitABC(OP_GETTABLE, a, b, c);
}

// r[a][rk(b)] = rk(c)
void funcInfo::emitSetTable(int a, int b, int c) {
    this->emitABC(OP_SETTABLE, a, b, c);
}

// r[a] = upval[b]
void funcInfo::emitGetUpval(int a, int b) {
    this->emitABC(OP_GETUPVAL, a, b, 0);
}

// upval[b] = r[a]
void funcInfo::emitSetUpval(int a, int b) {
    this->emitABC(OP_SETUPVAL, a, b, 0);
}

// r[a] = upval[b][rk(c)]
void funcInfo::emitGetTabUp(int a, int b, int c) {
    this->emitABC(OP_GETTABUP, a, b, c);
}

// upval[a][rk(b)] = rk(c)
void funcInfo::emitSetTabUp(int a, int b, int c) {
    this->emitABC(OP_SETTABUP, a, b, c);
}

// r[a], ..., r[a+c-2] = r[a](r[a+1], ..., r[a+b-1])
void funcInfo::emitCall(int a, int nArgs, int nRet) {
    this->emitABC(OP_CALL, a, nArgs+1, nRet+1);
}

// return r[a](r[a+1], ... ,r[a+b-1])
void funcInfo::emitTailCall(int a, int nArgs) {
    this->emitABC(OP_TAILCALL, a, nArgs+1, 0);
}

// return r[a], ... ,r[a+b-2]
void funcInfo::emitReturn(int a, int n) {
    this->emitABC(OP_RETURN, a, n+1, 0);
}

// r[a+1] := r[b]; r[a] := r[b][rk(c)]
void funcInfo::emitSelf(int a, int b, int c) {
    this->emitABC(OP_SELF, a, b, c);
}

// pc+=sBx; if (a) close all upvalues >= r[a - 1]
int funcInfo::emitJmp(int a, int sBx)  {
    this->emitAsBx(OP_JMP, a, sBx);
    return static_cast<int>(this->insts->size()) - 1;
}

// if not (r[a] <=> c) then pc++
void funcInfo::emitTest(int a, int c) {
    this->emitABC(OP_TEST, a, 0, c);
}

// if (r[b] <=> c) then r[a] := r[b] else pc++
void funcInfo::emitTestSet(int a, int b, int c) {
    this->emitABC(OP_TESTSET, a, b, c);
}

int funcInfo::emitForPrep(int a, int sBx)  {
    this->emitAsBx(OP_FORPREP, a, sBx);
    return static_cast<int>(this->insts->size()) - 1;
}

int funcInfo::emitForLoop(int a, int sBx)  {
    this->emitAsBx(OP_FORLOOP, a, sBx);
    return static_cast<int>(this->insts->size()) - 1;
}

void funcInfo::emitTForCall(int a, int c) {
this->emitABC(OP_TFORCALL, a, 0, c);
}

void funcInfo::emitTForLoop(int a, int sBx) {
    this->emitAsBx(OP_TFORLOOP, a, sBx);
}

void funcInfo::emitUnaryOp(int op, int a, int b){
    switch (op) {
        case TOKEN_OP_NOT:
            this->emitABC(OP_NOT, a, b, 0);
            break;
        case TOKEN_OP_BNOT:
            this->emitABC(OP_BNOT, a, b, 0);
            break;
        case TOKEN_OP_LEN:
            this->emitABC(OP_LEN, a, b, 0);
            break;
        case TOKEN_OP_UNM:
            this->emitABC(OP_UNM, a, b, 0);
            break;
        default:
            std::cerr << "Error! emitUnaryOp, op = " << op << std::endl;
            abort();
    }
}


void funcInfo::emitBinaryOp(int op, int a, int b, int c){
    if (arithAndBitwiseBinops.find(op) != arithAndBitwiseBinops.end()){
        this->emitABC(arithAndBitwiseBinops.at(op), a, b, c);
    } else {
        switch (op) {
            case TOKEN_OP_EQ:
                this->emitABC(OP_EQ, 1, b, c); break;
            case TOKEN_OP_NE:
                this->emitABC(OP_EQ, 0, b, c); break;
            case TOKEN_OP_LT:
                this->emitABC(OP_LT, 1, b, c); break;
            case TOKEN_OP_GT:
                this->emitABC(OP_LT, 1, c, b); break;
            case TOKEN_OP_LE:
                this->emitABC(OP_LE, 1, b, c); break;
            case TOKEN_OP_GE:
                this->emitABC(OP_LE, 1, c, b); break;
            default:
                std::cerr << "Error! Location: funcInfo::emitBinaryOp, Op = " << op << std::endl;
                abort();
        }
        this->emitJmp(0, 1);
        this->emitLoadBool(a, 0, 1);
        this->emitLoadBool(a, 1, 0);
    }
}

Prototype * toProto(funcInfo * fi){
    auto proto = new Prototype();
    proto->NumParams = static_cast<byte>(fi->numParams);
    proto->MaxStackSize = static_cast<byte>(fi->maxRegs);
    proto->Code = fi->insts;
    proto->Constants = getConstants(fi);
    proto->Upvalues = getUpvalues(fi);
    proto->Protos = toProtos(fi->subFuncs);
    proto->LineInfo = new std::vector<uint32_t>();
    proto->LocVars = new std::vector<LocVar>();
    proto->UpvalueNames = new std::vector<std::string>();
    if (fi->isVararg) {proto->IsVararg = 1;};
    return proto;
}

std::vector<Prototype *> * toProtos(std::vector<funcInfo *> * fis) {
    auto protos = new std::vector<Prototype*>(fis->size(), nullptr);
    for(auto i = 0; i < fis->size(); i++){
        protos->at(i) =  toProto(fis->at(i));
    }
    return protos;
}

std::vector<luaValue> * getConstants(funcInfo * fi){
    auto consts = new std::vector<luaValue>(fi->constants->size());
    for(auto & [k, idx] : *fi->constants){
        consts->at(idx) = k;
    }
    return consts;
}

std::vector<Upvalue> * getUpvalues(funcInfo * fi){
    auto upvals = new std::vector<Upvalue>(fi->upvalues->size());
    for(auto & [_, uv] : *fi->upvalues){
        if (uv->locVarSlot >= 0){
            upvals->at(uv->index) = Upvalue{1, static_cast<byte>(uv->locVarSlot)};
        } else {
            upvals->at(uv->index) = Upvalue{0, static_cast<byte>(uv->upvalIndex)};
        }
    }
    return upvals;
}
