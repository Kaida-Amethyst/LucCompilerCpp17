//
// Created by ziyue on 2021/6/3.
//

#ifndef CH15_BLOCK_H
#define CH15_BLOCK_H

#include <utility>
#include <vector>
#include <string>
#include <cstdint>
#include <any>




enum ExpType{
    EXP_NIL, EXP_TRUE, EXP_FALSE, EXP_VARARG, EXP_INTEGER, EXP_FLOAT, EXP_STRING,
    EXP_NAME, EXP_UNOP, EXP_BINOP, EXP_CONCAT, EXP_TABLE_CONSTRUCTOR, EXP_FUNC_DEF,
    EXP_PARENS, EXP_TABLE_ACCESS, EXP_FUNC_CALL
};

class Exp{
public:
    virtual ExpType type() const noexcept = 0;
//    virtual ExpType type() const noexcept {return EXP_UNKNOWN; };
};

class Block;



struct NilExp : public Exp{
    int Line;
    explicit NilExp(int line):Line{line}{};
    ExpType type() const noexcept {return EXP_NIL;}
};

struct TrueExp : public Exp{
    int Line;
    explicit TrueExp(int line):Line{line}{};
    ExpType type() const noexcept {return EXP_TRUE;}
};

struct FalseExp : public Exp{
    int Line;
    explicit FalseExp(int line):Line{line}{};
    ExpType type() const noexcept {return EXP_FALSE;}
};

struct VarargExp : public Exp{
    int Line;
    explicit VarargExp(int line):Line{line}{};
    ExpType type() const noexcept {return EXP_VARARG;}
};

struct IntegerExp : public Exp{
    int Line; int64_t Val;
    IntegerExp(int line, int64_t val):Line{line}, Val{val}{}
    ExpType type() const noexcept {return EXP_INTEGER;}
};

struct FloatExp : public Exp{
    int Line; double Val;
    FloatExp(int line, double val):Line{line}, Val{val}{}
    ExpType type() const noexcept {return EXP_FLOAT;}
};

struct StringExp : public Exp{
    int Line; std::string Str;
    StringExp(int line, std::string str):Line{line}, Str{std::move(str)}{}
    ExpType type() const noexcept {return EXP_STRING;}
};

struct NameExp : public Exp{
    int Line; std::string Name;
    NameExp(int line, std::string name):Line(line), Name(std::move(name)){}
    ExpType type() const noexcept {return EXP_NAME;}
};

struct UnopExp : public Exp{
    int Line;
    int Op;
    Exp * exp;
    UnopExp(int line, int op, Exp * e):Line{line}, Op{op}, exp{e}{};
    ExpType type() const noexcept {return EXP_UNOP;}
};

struct BinopExp : public Exp{
    int Line;
    int Op;
    Exp * Exp1;
    Exp * Exp2;
    BinopExp(int line, int op, Exp * e1, Exp * e2):Line{line}, Op{op}, Exp1{e1}, Exp2{e2}{};
    ExpType type() const noexcept {return EXP_BINOP;}
};

struct ConcatExp : public Exp{
    int Line;
    std::vector<Exp*> * exps;
    ConcatExp(int line, std::vector<Exp*> * es) : Line{line}, exps{es}{}
    ExpType type() const noexcept {return EXP_CONCAT;}
};

struct TableConstructorExp : public Exp{
    int Line;
    int LastLine;
    std::vector<Exp*> * KeyExps;
    std::vector<Exp*> * ValExps;
    TableConstructorExp(int line, int lastline, std::vector<Exp*> * kes,  std::vector<Exp*> * ves):
    Line{line}, LastLine{lastline}, KeyExps{kes}, ValExps{ves}{}
    ExpType type() const noexcept {return EXP_TABLE_CONSTRUCTOR;}
};

struct FuncDefExp : public Exp{
    int Line;
    int LastLine;
    std::vector<std::string> * ParList;
    bool IsVararg;
    Block * block;
    FuncDefExp(int line, int lastline, std::vector<std::string> * parList, bool isvararg, Block * b)
    :Line(line), LastLine{lastline}, ParList{parList}, IsVararg{isvararg}, block{b}{};
    ExpType type() const noexcept {return EXP_FUNC_DEF;}
};

struct ParensExp: public Exp{
    Exp * exp;
    explicit ParensExp(Exp * e):exp{e}{}
    ExpType type() const noexcept {return EXP_PARENS;}
};

struct TableAccessExp : public Exp{
    int Line;
    Exp * PrefixExp;
    Exp * KeyExp;
    TableAccessExp(int line, Exp * pe, Exp * ke):Line{line}, PrefixExp{pe}, KeyExp{ke}{}
    ExpType type() const noexcept {return EXP_TABLE_ACCESS;}
};

struct FuncCallExp : public Exp{
    int Line;
    int LastLine;
    Exp * PrefixExp;
    StringExp * NameExp;
    std::vector<Exp*> * Args;
    FuncCallExp(int line, int lastline, Exp * pe, StringExp * ne, std::vector<Exp*> * args):
    Line{line}, LastLine{lastline}, PrefixExp{pe}, NameExp{ne}, Args{args}{};
    ExpType type() const noexcept {return EXP_FUNC_CALL;}
};

enum StatType{
    STAT_EMPTY, STAT_BREAK, STAT_LABEL, STAT_GOTO, STAT_DO, STAT_FUNC_CALL, STAT_WHILE,
    STAT_REPEAT, STAT_IF, STAT_FOR_NUM, STAT_FOR_IN, STAT_LOCAL_VAR_DECL,  STAT_ASSIGN,
    STAT_LOCAL_FUNC_DEF
};

class Stat{
public:
    virtual StatType type() const noexcept = 0;
};

struct EmptyStat : public Stat{
    StatType type() const noexcept {return STAT_EMPTY;};
};

struct BreakStat :public Stat{
    int Line;
    explicit BreakStat(int l):Line{l}{};
    StatType type() const noexcept {return STAT_BREAK;};
};

struct LabelStat : public Stat{
    std::string Name;
    explicit LabelStat(std::string  n):Name(std::move(n)){}
    StatType type() const noexcept {return STAT_LABEL;};
};

struct GotoStat : public Stat{
    std::string Name;
    explicit GotoStat(std::string  n):Name{std::move(n)}{}
    StatType type() const noexcept {return STAT_GOTO;};
};

struct DoStat : public Stat{
    Block * block;
    explicit DoStat(Block * b):block(b){};
    StatType type() const noexcept {return STAT_DO;};
};

struct FuncCallStat  : public Stat{
    FuncCallExp * fcexp;
    explicit FuncCallStat(FuncCallExp * fce):fcexp{fce}{}
    StatType type() const noexcept {return STAT_FUNC_CALL;};
};

struct WhileStat : public Stat{
    Exp * exp;
    Block * block;
    WhileStat(Exp * e, Block * b):exp(e), block(b){}
    StatType type() const noexcept {return STAT_WHILE;};
};

struct RepeatStat : public Stat{
    Block * block;
    Exp * exp;
    RepeatStat(Block * b, Exp * e):block(b), exp(e){}
    StatType type() const noexcept {return STAT_REPEAT;};
};

struct IfStat : public Stat{
    std::vector<Exp*> * Exps;
    std::vector<Block*> * blocks;
    IfStat(std::vector<Exp*> * es, std::vector<Block*> * bs):Exps(es), blocks(bs){}
    StatType type() const noexcept {return STAT_IF;};
};

struct ForNumStat : public Stat{
    int LineOfFor;
    int LineOfDo;
    std::string VarName;
    Exp * InitExp;
    Exp * LimitExp;
    Exp * StepExp;
    Block * block;
    ForNumStat(int lf, int ld, std::string vn, Exp * ie, Exp * le, Exp * se, Block * b):
    LineOfFor{lf}, LineOfDo{ld}, VarName(std::move(vn)), InitExp(ie), LimitExp(le), StepExp(se), block(b){}
    StatType type() const noexcept {return STAT_FOR_NUM;};
};

struct ForInStat : public Stat{
    int LineOfDo;
    std::vector<std::string> * NameList;
    std::vector<Exp*> * ExpList;
    Block * block;
    ForInStat(int lf, std::vector<std::string> * nl, std::vector<Exp*> * el, Block * b):
    LineOfDo(lf), NameList{nl}, ExpList{el}, block{b}{}
    StatType type() const noexcept {return STAT_FOR_IN;};
};

struct LocalVarDeclStat : public Stat{
    int LastLine;
    std::vector<std::string> * NameList;
    std::vector<Exp*> * ExpList;
    LocalVarDeclStat(int ll, std::vector<std::string> * nl, std::vector<Exp*> * el)
    :LastLine(ll), NameList(nl), ExpList(el){}
    StatType type() const noexcept {return STAT_LOCAL_VAR_DECL;};
};

struct AssignStat : public Stat{
    int LastLine;
    std::vector<Exp*> * VarList;
    std::vector<Exp*> * ExpList;
    AssignStat(int ll, std::vector<Exp*> * vl, std::vector<Exp*> * el):
    LastLine(ll), VarList(vl), ExpList(el){}
    StatType type() const noexcept {return STAT_ASSIGN;};
};

struct LocalFuncDefStat : public Stat{
    std::string Name;
    FuncDefExp * fde;
//    Exp * FuncDefExp;
    LocalFuncDefStat(std::string n, Exp * fde):Name(std::move(n)), fde{dynamic_cast<FuncDefExp*>(fde)}{}
    StatType type() const noexcept {return STAT_LOCAL_FUNC_DEF;};
};

class Block{
public:
    int LastLine;
    std::vector<Stat*> * Stats;
    std::vector<Exp*> * RetExps;
    Block(int ll, std::vector<Stat*> * s, std::vector<Exp*> *r):LastLine{ll},Stats{s}, RetExps{r}{};
};

#endif //CH15_BLOCK_H
