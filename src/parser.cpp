//
// Created by ziyue on 2021/6/4.
//

#include "parser.h"

#include <utility>
#include <iostream>
#include "token.h"
#include "optimizer.h"


std::pair<int, bool> toInteger(const std::string &st){
    if (st.empty() || !std::isdigit(st.at(0))){
        return std::pair{-1, false};
    }
    size_t p = 0;
    int res = std::stoi(st, &p);
    return std::pair{res, (p == st.size())};
}

std::pair<double, bool> toFloat(const std::string &st){
    if (st.empty() || (!std::isdigit(st.at(0)) && st.at(0) != '.')){
        return std::pair{-1, false};
    }
    size_t p = 0;
    double res = std::stod(st, &p);
    return std::pair{res, (p == st.size())};
}

Block * parse(const std::string & chunk, const std::string & chunkName){
    Lexer lexer(chunkName, chunk);
    return parseBlock(lexer);
}

Block * parseBlock(Lexer & lexer){
    auto * block = new Block(lexer.Line(), parseStats(lexer), parseRetExps(lexer));
    return block;
}

std::vector<Stat*> *  parseStats(Lexer & lexer){
    auto * stats = new std::vector<Stat*>{};
    while(! _isReturnOrBlockEnd(lexer.LookAhead())){
        auto * stat = parseStat(lexer);
        if (stat->type() != STAT_EMPTY){
            stats->push_back(stat);
        }
    }
    return stats;
}

inline bool _isReturnOrBlockEnd(int tokenKind){
    switch (tokenKind) {
        case TOKEN_KW_RETURN :
        case TOKEN_EOF :
        case TOKEN_KW_END:
        case TOKEN_KW_ELSE:
        case TOKEN_KW_ELSEIF:
        case TOKEN_KW_UNTIL:
            return true;
        default:return false;
    }
}

std::vector<Exp*> * parseExpList(Lexer & lexer){
    auto exps = new std::vector<Exp*>{};
    exps->push_back(parseExp(lexer));
    while(lexer.LookAhead() == TOKEN_SEP_COMMA){
        lexer.NextToken();
        exps->push_back(parseExp(lexer));
    }
    return exps;
}


std::vector<Exp*> *  parseRetExps(Lexer & lexer){
    if (lexer.LookAhead() != TOKEN_KW_RETURN){ return nullptr;}
    lexer.NextToken();
    switch (lexer.LookAhead()) {
        case TOKEN_EOF:
        case TOKEN_KW_END:
        case TOKEN_KW_ELSE:
        case TOKEN_KW_ELSEIF:
        case TOKEN_KW_UNTIL:
            return new std::vector<Exp*>{};
        case TOKEN_SEP_SEMI:
            lexer.NextToken();
            return new std::vector<Exp*>{};
        default:
            auto exps = parseExpList(lexer);
            if (lexer.LookAhead() == TOKEN_SEP_SEMI){
                lexer.NextToken();
            }
            return exps;
    }
}

Stat * parseStat(Lexer & lexer){
    switch (lexer.LookAhead()) {
        case TOKEN_SEP_SEMI:
            return parseEmptyStat(lexer);
        case TOKEN_KW_BREAK:
            return parseBreakStat(lexer);
        case TOKEN_SEP_LABEL:
            return parseLabelStat(lexer);
        case TOKEN_KW_GOTO:
            return parseGotoStat(lexer);
        case TOKEN_KW_DO:
            return parseDoStat(lexer);
        case TOKEN_KW_WHILE:
            return parseWhileStat(lexer);
        case TOKEN_KW_REPEAT:
            return parseRepeatStat(lexer);
        case TOKEN_KW_IF:
            return parseIfStat(lexer);
        case TOKEN_KW_FOR:
            return parseForStat(lexer);
        case TOKEN_KW_FUNCTION:
            return parseFuncDefStat(lexer);
        case TOKEN_KW_LOCAL:
            return parseLocalAssignOrFuncDefStat(lexer);
        default:
            return parseAssignOrFuncCallStat(lexer);
    }
}

EmptyStat * parseEmptyStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_SEP_SEMI);
    return new EmptyStat{};
}

BreakStat * parseBreakStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_KW_BREAK);
    auto s = new BreakStat{lexer.Line()};
    return s;
}

LabelStat * parseLabelStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_SEP_LABEL);
    auto [_, name] = lexer.NextIdentifier();
    lexer.NextTokeOfKind(TOKEN_SEP_LABEL);
    auto s = new LabelStat{name};
    return s;
}

GotoStat * parseGotoStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_KW_GOTO);
    auto [_, name] = lexer.NextIdentifier();
    auto s = new GotoStat{name};
    return s;
}

DoStat * parseDoStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_KW_DO);
    auto * block = parseBlock(lexer);
    lexer.NextTokeOfKind(TOKEN_KW_END);
    auto s = new DoStat{block};
    return s;
}

WhileStat * parseWhileStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_KW_WHILE);
    auto * exp = parseExp(lexer);
    lexer.NextTokeOfKind(TOKEN_KW_DO);
    auto * block = parseBlock(lexer);
    lexer.NextTokeOfKind(TOKEN_KW_END);
    auto s = new WhileStat{exp, block};
    return s;
}

RepeatStat * parseRepeatStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_KW_REPEAT);
    auto * block = parseBlock(lexer);
    lexer.NextTokeOfKind(TOKEN_KW_UNTIL);
    auto * exp = parseExp(lexer);
    auto s = new RepeatStat{block, exp};
    return s;
}

IfStat * parseIfStat(Lexer & lexer){
    auto exps = new std::vector<Exp*>{};
    auto blocks = new std::vector<Block*>{};
    lexer.NextTokeOfKind(TOKEN_KW_IF);
    exps->push_back(parseExp(lexer));
    lexer.NextTokeOfKind(TOKEN_KW_THEN);
    blocks->push_back(parseBlock(lexer));
    while(lexer.LookAhead() == TOKEN_KW_ELSEIF){
        lexer.NextToken();
        exps->push_back(parseExp(lexer));
        lexer.NextTokeOfKind(TOKEN_KW_THEN);
        blocks->push_back(parseBlock(lexer));
    }
    if (lexer.LookAhead() == TOKEN_KW_ELSE){
        lexer.NextToken();
        auto e = new TrueExp{lexer.Line()};
        exps->push_back(e);
        blocks->push_back(parseBlock(lexer));
    }
    lexer.NextTokeOfKind(TOKEN_KW_END);
    auto s = new IfStat{exps, blocks};
    return s;
}

ForNumStat * _finishForNumStat(Lexer & lexer, int lineOfFor, std::string varName){
    lexer.NextTokeOfKind(TOKEN_OP_ASSIGN);
    auto initExp = parseExp(lexer);
    lexer.NextTokeOfKind(TOKEN_SEP_COMMA);
    auto limitExp = parseExp(lexer);
    Exp * stepExp;
    if (lexer.LookAhead() == TOKEN_SEP_COMMA){
        lexer.NextToken();
        stepExp = parseExp(lexer);
    }else{
        auto e = new IntegerExp{lexer.Line(), 1};
        stepExp = e;
    }
    auto [lineOfDo, _] = lexer.NextTokeOfKind(TOKEN_KW_DO);
    auto block = parseBlock(lexer);
    lexer.NextTokeOfKind(TOKEN_KW_END);
    auto s = new ForNumStat{lineOfFor, lineOfDo, std::move(varName), initExp, limitExp, stepExp, block};
    return s;
}

std::vector<std::string> * _finishNameList(Lexer & lexer, const std::string & name0){
    auto names = new std::vector<std::string>{};
    names->push_back(name0);
    while(lexer.LookAhead() == TOKEN_SEP_COMMA){
        lexer.NextToken();
        auto [_, name] = lexer.NextIdentifier();
        names->push_back(name);
    }
    return names;
}

ForInStat * _finishForInStat(Lexer & lexer, const std::string & name0){
    auto nameList = _finishNameList(lexer, name0);
    lexer.NextTokeOfKind(TOKEN_KW_IN);
    auto expList = parseExpList(lexer);
    auto [lineOfDo, _] = lexer.NextTokeOfKind(TOKEN_KW_DO);
    auto block = parseBlock(lexer);
    lexer.NextTokeOfKind(TOKEN_KW_END);
    auto s = new ForInStat{lineOfDo, nameList, expList, block};
    return s;
}

Stat * parseForStat(Lexer & lexer){
    auto [lineOfFor, _] = lexer.NextTokeOfKind(TOKEN_KW_FOR);
    auto [_t, name] = lexer.NextIdentifier();
    if (lexer.LookAhead() == TOKEN_OP_ASSIGN){
        return _finishForNumStat(lexer, lineOfFor, name);
    }else{
        return _finishForInStat(lexer, name);
    }
}

LocalFuncDefStat * _finishLocalFuncDefStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_KW_FUNCTION);
    auto [_, name] = lexer.NextIdentifier();
    auto fdExp = parseFuncDefExp(lexer);
    auto s = new LocalFuncDefStat{std::move(name), fdExp};
    return s;
}

LocalVarDeclStat * _finishLocalVarDeclStat(Lexer & lexer){
    auto [_,name0] = lexer.NextIdentifier();
    auto nameList = _finishNameList(lexer, name0);
    std::vector<Exp*> * expList = nullptr;
    if (lexer.LookAhead() == TOKEN_OP_ASSIGN){
        lexer.NextToken();
        expList = parseExpList(lexer);
    }
    auto lastLine = lexer.Line();
    auto s = new LocalVarDeclStat{lastLine, nameList, expList};
    return s;
}

Stat * parseLocalAssignOrFuncDefStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_KW_LOCAL);
    if (lexer.LookAhead() == TOKEN_KW_FUNCTION){
        return _finishLocalFuncDefStat(lexer);
    }else{
        return _finishLocalVarDeclStat(lexer);
    }
}

std::vector<Exp*> * _finishVarList(Lexer & lexer, Exp * var0){
    auto vars = new std::vector<Exp*>{};
    vars->push_back(_checkVar(lexer, var0));
    while(lexer.LookAhead() == TOKEN_SEP_COMMA){
        lexer.NextToken();
        auto exp = parsePrefixExp(lexer);
        vars->push_back(_checkVar(lexer, exp));
    }
    return vars;
}

Exp * _checkVar(Lexer & lexer, Exp * exp){
    if (exp->type() == EXP_NAME || exp->type() == EXP_TABLE_ACCESS){
        return exp;
    }
    lexer.NextTokeOfKind(-1);
    std::cerr << "Unreachable!" << std::endl;
    abort();
}


AssignStat * parseAssignStat(Lexer & lexer, Exp * var0){
    auto varList = _finishVarList(lexer, var0);
    lexer.NextTokeOfKind(TOKEN_OP_ASSIGN);
    auto expList = parseExpList(lexer);
    auto lastLine = lexer.Line();
    auto s = new AssignStat{lastLine, varList, expList};
    return s;
}

Stat * parseAssignOrFuncCallStat(Lexer & lexer){
    auto prefixExp = parsePrefixExp(lexer);
    if (prefixExp->type() == EXP_FUNC_CALL){
        auto pe = dynamic_cast<FuncCallExp*>(prefixExp);
        auto fc = new FuncCallStat{pe};
        return fc;
    }else{
        return parseAssignStat(lexer, prefixExp);
    }
}

std::pair<Exp*, bool> _parseFuncName(Lexer & lexer){
    auto [line, name] = lexer.NextIdentifier();
    Exp * exp = new NameExp{line, name};;
    bool hasColon = false;
    while(lexer.LookAhead() == TOKEN_SEP_DOT){
        lexer.NextToken();
        auto [line, name] = lexer.NextIdentifier();
        auto idx = new StringExp{line, name};
        exp = new TableAccessExp{line, exp, idx};
    }
    if (lexer.LookAhead() == TOKEN_SEP_COLON){
        lexer.NextToken();
        auto [line, name] = lexer.NextIdentifier();
        auto idx = new StringExp{line, name};
        exp = new TableAccessExp{line, exp, idx};
        hasColon = true;
    }
    return std::pair{exp, hasColon};
}

// Problematic
AssignStat * parseFuncDefStat(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_KW_FUNCTION);
    auto [fnExp, hasColon] = _parseFuncName(lexer);
    auto fdExp = parseFuncDefExp(lexer);
    if (hasColon){
        fdExp->ParList->push_back("");
        fdExp->ParList->insert(fdExp->ParList->begin(), "self");
    }
    auto s = new AssignStat{fdExp->Line, new std::vector<Exp*>{fnExp},new std::vector<Exp*>{fdExp}};
    return s;
}



Exp * parseExp0(Lexer & lexer){
    int line;
    std::string token;

    switch (lexer.LookAhead()) {
        case TOKEN_VARARG:
            line = std::get<0>(lexer.NextToken());
            return new VarargExp{line};
        case TOKEN_KW_NIL:
            line = std::get<0>(lexer.NextToken());
            return new NilExp{line};
        case TOKEN_KW_TRUE:
            line = std::get<0>(lexer.NextToken());
            return new TrueExp{line};
        case TOKEN_KW_FALSE:
            line = std::get<0>(lexer.NextToken());
            return new FalseExp{line};
        case TOKEN_STRING:{
            auto tt = lexer.NextToken();
            line = std::get<0>(tt);
            token = std::get<2>(tt);
            return new StringExp{line, token};
        }
        case TOKEN_NUMBER:
            return parseNumberExp(lexer);
        case TOKEN_SEP_LCURLY:
            return parseTableConstructorExp(lexer);
        case TOKEN_KW_FUNCTION:
            lexer.NextToken();
            return parseFuncDefExp(lexer);
        default:
            return parsePrefixExp(lexer);
    }    
}

Exp * parseNumberExp(Lexer & lexer){
    auto [line, _, token] = lexer.NextToken();
    if (auto [i, ok] = toInteger(token); ok){
        return new IntegerExp{line, i};
    }else if (auto [f, ok] = toFloat(token); ok){
        return new FloatExp{line, f};
    }else{
        std::cerr << token << " is Not a Number!" << " line : " << line << std::endl;
        abort();
    }
}

std::pair<std::vector<std::string> *, bool> _parseParList(Lexer & lexer){
    switch (lexer.LookAhead()) {
        case TOKEN_SEP_RPAREN:
            return std::pair{nullptr, false};
        case TOKEN_VARARG:
            lexer.NextToken();
            return std::pair{nullptr, true};
    }
    auto * names = new std::vector<std::string> {};
    bool isVararg = false;
    auto name = lexer.NextIdentifier().second;
    names->push_back(name);
    while(lexer.LookAhead() == TOKEN_SEP_COMMA){
        lexer.NextToken();
        if (lexer.LookAhead() == TOKEN_IDENTIFIER){
            name = lexer.NextIdentifier().second;
            names->push_back(name);
        }else{
            lexer.NextTokeOfKind(TOKEN_VARARG);
            isVararg = true;
            break;
        }
    }
    return std::pair{names, isVararg};
}

FuncDefExp * parseFuncDefExp(Lexer & lexer){
    auto line = lexer.Line();
    lexer.NextTokeOfKind(TOKEN_SEP_LPAREN);
    auto [parList, isVararg] = _parseParList(lexer);
    lexer.NextTokeOfKind(TOKEN_SEP_RPAREN);
    auto block = parseBlock(lexer);
    auto [lastLine, _] = lexer.NextTokeOfKind(TOKEN_KW_END);
    return new FuncDefExp{line, lastLine, parList, isVararg, block};
}

inline bool _isFieldSep(int tokenKind){
    return tokenKind == TOKEN_SEP_COMMA || tokenKind == TOKEN_SEP_SEMI;
}

std::pair<Exp*, Exp*> _parseField(Lexer & lexer){
    Exp * k;
    Exp * v;
    if (lexer.LookAhead() == TOKEN_SEP_LBRACK){
        lexer.NextToken();
        k = parseExp(lexer);
        lexer.NextTokeOfKind(TOKEN_SEP_RBRACK);
        lexer.NextTokeOfKind(TOKEN_OP_ASSIGN);
        v = parseExp(lexer);
        return std::pair{k, v};
    }
    auto exp = parseExp(lexer);
    if (exp->type() == EXP_NAME){
        auto * ne = dynamic_cast<NameExp*>(exp);
        if (lexer.LookAhead() == TOKEN_OP_ASSIGN){
            lexer.NextToken();
            k = new StringExp{ne->Line, ne->Name};
            v = parseExp(lexer);
            return std::pair{k, v};
        }
    }
    return std::pair{nullptr, exp};
}

std::pair<std::vector<Exp*> *, std::vector<Exp*> *> _parseFieldList(Lexer & lexer){
    auto * ks = new std::vector<Exp*>{};
    auto * vs = new std::vector<Exp*>{};
    if (lexer.LookAhead() != TOKEN_SEP_RCURLY){
        auto [k, v] = _parseField(lexer);
        ks->push_back(k); vs->push_back(v);
        while(_isFieldSep(lexer.LookAhead())){
            lexer.NextToken();
            if (lexer.LookAhead() != TOKEN_SEP_RCURLY){
                auto [k, v] = _parseField(lexer);
                ks->push_back(k); vs->push_back(v);
            }else{
                break;
            }
        }
    }
    return std::pair{ks, vs};
}

TableConstructorExp * parseTableConstructorExp(Lexer & lexer){
    auto line = lexer.Line();
    lexer.NextTokeOfKind(TOKEN_SEP_LCURLY);
    auto [keyExps,valExps] = _parseFieldList(lexer);
    lexer.NextTokeOfKind(TOKEN_SEP_RCURLY);
    auto lastLine = lexer.Line();
    return new TableConstructorExp{line, lastLine, keyExps, valExps};
}

Exp * _finishPrefixExp(Lexer & lexer, Exp * exp){
    while(true){
        switch (lexer.LookAhead()) {
            case TOKEN_SEP_LBRACK:{
                lexer.NextToken();
                auto keyExp = parseExp(lexer);
                lexer.NextTokeOfKind(TOKEN_SEP_RBRACK);
                exp = new TableAccessExp{lexer.Line(), exp, keyExp};
                continue;
            }
            case TOKEN_SEP_DOT:{
                lexer.NextToken();
                auto [line, name] = lexer.NextIdentifier();
                auto keyExp = new StringExp{line, name};
                exp = new TableAccessExp{line, exp, keyExp};
                continue;
            }
            case TOKEN_SEP_COLON:
            case TOKEN_SEP_LPAREN:
            case TOKEN_SEP_LCURLY:
            case TOKEN_STRING:
                exp = _finishFuncCallExp(lexer, exp);
                continue;
            default:
                return exp;
        }
    }
    return exp;
}

Exp * parseParensExp(Lexer & lexer){
    lexer.NextTokeOfKind(TOKEN_SEP_LPAREN);
    auto * exp = parseExp(lexer);
    lexer.NextTokeOfKind(TOKEN_SEP_RPAREN);

    if (exp->type() == EXP_VARARG
    || exp->type() == EXP_FUNC_CALL
    || exp->type() == EXP_NAME
    || exp->type() == EXP_TABLE_ACCESS){
        return new ParensExp(exp);
    }
    return exp;
}


StringExp * _parseNameExp(Lexer & lexer){
    if (lexer.LookAhead() == TOKEN_SEP_COLON){
        lexer.NextToken();
        auto [line, name] = lexer.NextIdentifier();
        return new StringExp{line, name};
    }
    return nullptr;
}

std::vector<Exp*> * _parseArgs(Lexer & lexer){
    std::vector<Exp*> * args;
    switch (lexer.LookAhead()) {
        case TOKEN_SEP_LPAREN:
            lexer.NextToken();
            if (lexer.LookAhead() != TOKEN_SEP_RPAREN){
                args = parseExpList(lexer);
            }
            lexer.NextTokeOfKind(TOKEN_SEP_RPAREN);
            break;
        case TOKEN_SEP_LCURLY:
            args = new std::vector<Exp*>{};
            args->push_back(parseTableConstructorExp(lexer));
            break;
        default:
            auto [line, str] = lexer.NextTokeOfKind(TOKEN_STRING);
            args = new std::vector<Exp*>{};
            args->push_back(new StringExp{line, str});
    }
    return args;
}

FuncCallExp * _finishFuncCallExp(Lexer & lexer, Exp * prefixExp){
    auto nameExp = _parseNameExp(lexer);
    auto line = lexer.Line();
    auto args = _parseArgs(lexer);
    auto lastLine = lexer.Line();
    return new FuncCallExp{line, lastLine, prefixExp, nameExp, args};
}

Exp * parsePrefixExp(Lexer & lexer){
    Exp * exp;
    if (lexer.LookAhead() == TOKEN_IDENTIFIER){
        auto [line, name] = lexer.NextIdentifier();
        exp = new NameExp{line, name};
    }else{
        exp = parseParensExp(lexer);
    }
    return _finishPrefixExp(lexer, exp);
}



Exp * parseExp1(Lexer & lexer){
    auto exp = parseExp0(lexer);
    if (lexer.LookAhead() == TOKEN_OP_POW){
        auto [line, op, _] = lexer.NextToken();
        exp = new BinopExp{line, op, exp, parseExp2(lexer)};
    }
    return exp;
}

Exp * parseExp2(Lexer & lexer){
    switch (lexer.LookAhead()) {
        case TOKEN_OP_UNM:
        case TOKEN_OP_BNOT:
        case TOKEN_OP_LEN:
        case TOKEN_OP_NOT:
            auto [line, op, _] = lexer.NextToken();
            auto exp = new UnopExp{line, op, parseExp2(lexer)};
            return Optimizer::unaryOp(exp);
    }
    return parseExp1(lexer);
}

// * / // %
Exp * parseExp3(Lexer & lexer){
    auto exp = parseExp2(lexer);
    while(true){
        switch (lexer.LookAhead()) {
            case TOKEN_OP_MUL:
            case TOKEN_OP_MOD:
            case TOKEN_OP_DIV:
            case TOKEN_OP_IDIV:{
                auto [line, op, _] = lexer.NextToken();
                auto arith = new BinopExp{line, op, exp, parseExp2(lexer)};
                exp = Optimizer::arithBinaryOp(arith);
                break;
            }
            default:
                return exp;
        }
    }
}

// x +/- b
Exp * parseExp4(Lexer & lexer){
    auto exp = parseExp3(lexer);
    while(true){
        switch (lexer.LookAhead()) {
            case TOKEN_OP_ADD:
            case TOKEN_OP_SUB:{
                auto [line, op, _] = lexer.NextToken();
                auto arith = new BinopExp{line, op, exp, parseExp3(lexer)};
                exp = Optimizer::arithBinaryOp(arith);
                break;
            }
            default:
                return exp;
        }
    }
}

// a .. b
Exp * parseExp5(Lexer & lexer){
    auto exp = parseExp4(lexer);
    if (lexer.LookAhead() != TOKEN_OP_CONCAT){
        return exp;
    }
    auto line = 0;
    auto exps = new std::vector<Exp*>{exp};
    while(lexer.LookAhead() == TOKEN_OP_CONCAT){
        exps->push_back(parseExp4(lexer));
    }
    return new ConcatExp{line, exps};
}

// << >>
Exp * parseExp6(Lexer & lexer){
    auto exp = parseExp5(lexer);
    while(true){
        switch (lexer.LookAhead()) {
            case TOKEN_OP_SHL:
            case TOKEN_OP_SHR:{
                auto [line, op, _] = lexer.NextToken();
                auto shx = new BinopExp{line, op, exp, parseExp5(lexer)};
                exp = Optimizer::bitWiseBinaryOp(shx);
                break;
            }
            default:
                return exp;
        }
    }
    return exp;
}

//  x & y
Exp * parseExp7(Lexer & lexer){
    auto exp = parseExp6(lexer);
    while(lexer.LookAhead() == TOKEN_OP_BAND){
        auto [line, op, _] = lexer.NextToken();
        auto band = new BinopExp{line, op, exp, parseExp6(lexer)};
        exp = Optimizer::bitWiseBinaryOp(band);
    }
    return exp;
}

// x ~ y
Exp * parseExp8(Lexer & lexer){
    auto exp = parseExp7(lexer);
    while(lexer.LookAhead() == TOKEN_OP_BXOR){
        auto [line, op, _] = lexer.NextToken();
        auto bxor = new BinopExp{line, op, exp, parseExp7(lexer)};
        exp = Optimizer::bitWiseBinaryOp(bxor);
    }
    return exp;
}



// x | y
Exp * parseExp9(Lexer & lexer){
    auto exp = parseExp8(lexer);
    while(lexer.LookAhead() == TOKEN_OP_BOR){
        auto [line, op, _] = lexer.NextToken();
        auto bor = new BinopExp{line, op, exp, parseExp8(lexer)};
        exp = Optimizer::bitWiseBinaryOp(bor);
    }
    return exp;
}

// compare
Exp * parseExp10(Lexer & lexer){
    auto exp = parseExp9(lexer);
    while(true){
        switch (lexer.LookAhead()) {
            case TOKEN_OP_LT:
            case TOKEN_OP_GT:
            case TOKEN_OP_NE:
            case TOKEN_OP_LE:
            case TOKEN_OP_GE:
            case TOKEN_OP_EQ:{
                auto [line, op, _] = lexer.NextToken();
                exp = new BinopExp{line, op, exp, parseExp9(lexer)};
                break;
            }
            default:
                return exp;
        }
    }
}


// a AND b
Exp * parseExp11(Lexer & lexer){
    auto exp = parseExp10(lexer);
    while(lexer.LookAhead() == TOKEN_OP_AND){
        auto [line, op, _] = lexer.NextToken();
        auto land = new BinopExp{line, op, exp, parseExp10(lexer)};
        exp = Optimizer::logicalAnd(land);
    }
    return exp;
}

// a OR b
Exp * parseExp12(Lexer & lexer){
    Exp * exp = parseExp11(lexer);;
    while(lexer.LookAhead() == TOKEN_OP_OR){
        auto [line, op, _] = lexer.NextToken();
        auto lor = new BinopExp{line, op, exp, parseExp11(lexer)};
        exp = Optimizer::logicalOr(lor);
    }
    return exp;
}

Exp * parseExp(Lexer & lexer){
    return parseExp12(lexer);
}
