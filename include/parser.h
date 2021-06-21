//
// Created by ziyue on 2021/6/4.
//

#ifndef CH16_PARSER_H
#define CH16_PARSER_H

#include "lexer.h"
#include "block.h"
#include <vector>

Block * parse(const std::string & chunk, const std::string & chunkName);
Block * parseBlock(Lexer & lexer);
std::vector<Stat*> * parseStats(Lexer & lexer);
std::vector<Exp*> * parseRetExps(Lexer & lexer);
Stat * parseStat(Lexer & lexer);
inline bool _isReturnOrBlockEnd(int tokenKind);
std::vector<Exp*> * parseExpList(Lexer & lexer);
Exp * parseExp(Lexer & lexer);
Exp * parseNumberExp(Lexer & lexer);
std::pair<std::vector<std::string> *, bool> _parseParList(Lexer & lexer);
FuncDefExp * parseFuncDefExp(Lexer & lexer);
std::pair<Exp*, Exp*> _parseField(Lexer & lexer);
std::pair<std::vector<Exp*> *, std::vector<Exp*> *> _parseFieldList(Lexer & lexer);
inline bool _isFieldSep(int tokenKind);
TableConstructorExp * parseTableConstructorExp(Lexer & lexer);
Exp * _finishPrefixExp(Lexer & lexer, Exp * exp);
Exp * parseParensExp(Lexer & lexer);
Exp * parsePrefixExp(Lexer & lexer);
StringExp * _parseNameExp(Lexer & lexer);
std::vector<Exp*> * _parseArgs(Lexer & lexer);
FuncCallExp * _finishFuncCallExp(Lexer & lexer, Exp * prefixExp);

EmptyStat * parseEmptyStat(Lexer & lexer);
BreakStat * parseBreakStat(Lexer & lexer);
LabelStat * parseLabelStat(Lexer & lexer);
GotoStat * parseGotoStat(Lexer & lexer);
DoStat * parseDoStat(Lexer & lexer);
WhileStat * parseWhileStat(Lexer & lexer);
RepeatStat * parseRepeatStat(Lexer & lexer);
IfStat * parseIfStat(Lexer & lexer);
ForNumStat * _finishForNumStat(Lexer & lexer, int lineOfFor, std::string varName);
ForInStat * _finishForInStat(Lexer & lexer, const std::string & name);
std::vector<std::string> * _finishNameList(Lexer & lexer, const std::string & name0);
Stat * parseForStat(Lexer & lexer);
AssignStat * parseFuncDefStat(Lexer & lexer);
std::pair<Exp*, bool> _parseFuncName(Lexer & lexer);
LocalFuncDefStat * _finishLocalFuncDefStat(Lexer & lexer);
LocalVarDeclStat * _finishLocalVarDeclStat(Lexer & lexer);
Stat * parseLocalAssignOrFuncDefStat(Lexer & lexer);
AssignStat * parseAssignStat(Lexer & lexer, Exp * var0);
std::vector<Exp*> * _finishVarList(Lexer & lexer, Exp * var0);
Exp * _checkVar(Lexer & lexer, Exp * exp);
Stat * parseAssignOrFuncCallStat(Lexer & lexer);

Exp * parseExp0(Lexer & lexer);
Exp * parseExp1(Lexer & lexer);
Exp * parseExp2(Lexer & lexer);
Exp * parseExp3(Lexer & lexer);
Exp * parseExp4(Lexer & lexer);
Exp * parseExp5(Lexer & lexer);
Exp * parseExp6(Lexer & lexer);
Exp * parseExp7(Lexer & lexer);
Exp * parseExp8(Lexer & lexer);
Exp * parseExp9(Lexer & lexer);
Exp * parseExp10(Lexer & lexer);
Exp * parseExp11(Lexer & lexer);
Exp * parseExp12(Lexer & lexer);


#endif //CH16_PARSER_H
