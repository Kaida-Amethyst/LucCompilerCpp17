//
// Created by ziyue on 2021/6/2.
//

#include "lexer.h"
#include "token.h"

//extern std::unordered_map<std::string, int> keyword;

#include <utility>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std::string_literals;

Lexer::Lexer(std::string chunkName, const std::string & chunkStr)
        :chunkName{std::move(chunkName)}, chunk{std::make_unique<std::string>(chunkStr)}, iter{chunk->begin()}, line{1}{};

bool Lexer::test(const std::string & p) {
    return p.end() == std::mismatch(p.begin(), p.end(), iter).first;
}

inline bool Lexer::isWhiteSpace(const char & c) {
    return c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ';
}

inline bool Lexer::isNewLine(const char & c) {
    return c == '\r' || c == '\n';
}


std::string Lexer::scanLongString() {
    std::regex_search(iter, chunk->cend(), match_result, reOpeningLongBracket);
    if (match_result.empty()){
        std::cerr << "Invalid long string delimiter near " << *iter << "line:" << line << std::endl;
        abort();
    }
    std::string openingLongBracket = match_result[0];
    std::string closingLongBracket = openingLongBracket;
    std::replace(closingLongBracket.begin(), closingLongBracket.end(), '[', ']');
    auto closingLongBracketIdx = chunk->find(closingLongBracket, iter - chunk->cbegin());
    if (closingLongBracketIdx == std::string::npos){
        std::cerr << "Unfinished long string or comment!, line :" << line << std::endl;
        abort();
    }
    size_t startPoint = iter+openingLongBracket.size()-chunk->cbegin();
    size_t nc  = closingLongBracketIdx - startPoint;
    std::string str = chunk->substr(startPoint, nc);
    next(static_cast<int>(openingLongBracket.size() + str.size() + closingLongBracket.size()));
    str = std::regex_replace(str, reNewLine, "\n");
    line += std::count(str.begin(), str.end(), '\n');
    if (!str.empty() && str.at(0) == '\n'){
        str = str.substr('\n');
    }
    return str;
}

std::string Lexer::scanShortString() {
    std::regex_search(iter, chunk->cend(), match_result, reShortStr);
    if (!match_result.empty()){
        std::string str = match_result[0];
        next(str.size());
        str = str.substr(1, str.size()-2);
        if (str.find('\\') != std::string::npos){
            std::regex_search(str, match_result, reNewLine);
            line += match_result.size();
            str = escape(str);
        }
        return str;
    }
    std::cerr << "Unfinished String, line : " << line << std::endl;
    return "";
}

std::string Lexer::escape(const std::string &full_str) {
    std::string_view str {full_str};
    std::ostringstream os;
    while(!str.empty()){
        if (str.at(0) != '\\'){
            os << str.at(0);
            str = str.substr(1);
            continue;
        }
        if (str.size() == 1){
            std::cerr << "unfinished string! Location: escape" << std::endl;
        }
        switch (str.at(1)) {
            case 'a':
                os << '\a'; str = str.substr(2);
                continue;
            case 'b':
                os << '\b'; str = str.substr(2);
                continue;
            case 'f':
                os << '\f'; str = str.substr(2);
                continue;
            case 'n':
                os << '\n'; str = str.substr(2);
                continue;
            case '\n':
                os << '\n'; str = str.substr(2);
                continue;
            case 'r':
                os << '\r'; str = str.substr(2);
                continue;
            case 't':
                os << '\t'; str = str.substr(2);
                continue;
            case 'v':
                os << '\v'; str = str.substr(2);
                continue;
            case '"':
                os << '"'; str = str.substr(2);
                continue;
            case '\'':
                os << '\''; str = str.substr(2);
                continue;
            case '\\':
                os << '\\'; str = str.substr(2);
                continue;
            case '0' ... '9':
            {
                auto ms = std::string{str};
                std::regex_search(ms, match_result, reDecEscapeSeq);
                if (!match_result.empty()){
                    auto d = std::stoi(match_result[0].str().substr(1));
                    if (d <= 0xFF) {
                        os << std::setfill('0') << std::setw(4) << std::hex << d;
                        str = str.substr(match_result[0].str().size());
                        continue;
                    }
                    std::cerr << "decimal escape too large near %s" << std::endl;
                    abort();
                }
            }
            case 'x':
            {
                auto ms = std::string{str};
                std::regex_search(ms, match_result, reHexEscapeSeq);
                if (!match_result.empty()){
                    auto d = std::stoi(match_result[0].str().substr(2), NULL, 16);
                    os << std::hex << std::setw(4) << std::setfill('0') << d;
                    str = str.substr(match_result[0].str().size());
                    continue;
                }
            }
            case 'u':
            {
                auto ms = std::string{str};
                std::regex_search(ms, match_result, reUnicodeEscapeSeq);
                auto d = std::stoi(match_result[0].str().substr(3, match_result[0].length()-1), NULL, 16);
                if (d <= 0x10FFFF){
                    os << std::hex << std::setw(4) << std::setfill('0') << d;
                    str = str.substr(match_result[0].str().size());
                    continue;
                }
            }
            case 'z':
                str = str.substr(2);
                while(str.size() > 0 && isWhiteSpace(str[0])){
                    str = str.substr(1);
                    continue;
                }
        }

    }
    return os.str();
}

void Lexer::skipComment() {
    next(2);
    if (test("[")){
        if(std::regex_search(iter, chunk->cend(), match_result, reOpeningLongBracket)){
            scanLongString();
            return;
        }
    }
    while(iter!=chunk->end() && !isNewLine(*iter)){
        next(1);
    }
}

void Lexer::skipWhiteSpaces() {
    while(iter!=chunk->end()){
        if ( test("--") ){
            skipComment();
        } else if (test("\r\n") || test("\n\r")){
            next(2);
            line+=1;
        } else if (isNewLine(*iter)){
            next(1);
            line+=1;
        } else if (isWhiteSpace(*iter)){
            next(1);
        } else {
            break;
        }
    }
}



std::tuple<int, int, std::string> Lexer::NextToken() {
    if (nextTokenLine > 0){
        auto thisLine = nextTokenLine;
        auto & kind = nextTokenKind;
        auto & token = nextToken;
        this->line = nextTokenLine;
        nextTokenLine = 0;
        return std::tuple{thisLine, kind, token};
    }
    skipWhiteSpaces();
    if (iter == chunk->end()) {
        return std::tuple{line, TOKEN_EOF, "EOF"s};
    }
    switch (*iter) {
        case ';' :
            next(1);
            return std::tuple{line, TOKEN_SEP_SEMI, ""};
        case ',':
            next(1);
            return std::tuple{line, TOKEN_SEP_COMMA, ""};
        case '(':
            next(1);
            return std::tuple{line, TOKEN_SEP_LPAREN, "("};
        case ')':
            next(1);
            return std::tuple{line, TOKEN_SEP_RPAREN, ")"};
        case ']':
            next(1);
            return std::tuple{line, TOKEN_SEP_RBRACK, "]"};
        case '{':
            next(1);
            return std::tuple{line, TOKEN_SEP_LCURLY, "{"};
        case '}':
            next(1);
            return std::tuple{line, TOKEN_SEP_RCURLY, "}"};
        case '+':
            next(1);
            return std::tuple{line, TOKEN_OP_ADD, "+"};
        case '-':
            next(1);
            return std::tuple{line, TOKEN_OP_MINUS, "-"};
        case '*':
            next(1);
            return std::tuple{line, TOKEN_OP_MUL, "*"};
        case '^':
            next(1);
            return std::tuple{line, TOKEN_OP_POW, "^"};
        case '%':
            next(1);
            return std::tuple{line, TOKEN_OP_MOD, "%"};
        case '&':
            next(1);
            return std::tuple{line, TOKEN_OP_BAND, "&"};
        case '|':
            next(1);
            return std::tuple{line, TOKEN_OP_BOR, "|"};
        case '#':
            next(1);
            return std::tuple{line, TOKEN_OP_LEN, "#"};
        case ':':
            if (test("::")) {
                next(2);
                return std::tuple{line, TOKEN_SEP_LABEL, "::"};
            } else {
                next(1);
                return std::tuple{line, TOKEN_SEP_COLON, ":"};
            }
        case '/':
            if (test("//")) {
                next(2);
                return std::tuple{line, TOKEN_OP_IDIV, "//"};
            } else {
                next(1);
                return std::tuple{line, TOKEN_OP_DIV, "/"};
            }
        case '~':
            if (test("~=")) {
                next(2);
                return std::tuple{line, TOKEN_OP_NE, "~="};
            } else {
                next(1);
                return std::tuple{line, TOKEN_OP_WAVE, "~"};
            }
        case '=':
            if (test("==")) {
                next(2);
                return std::tuple{line, TOKEN_OP_EQ, "=="};
            } else {
                next(1);
                return std::tuple{line, TOKEN_OP_ASSIGN, "="};
            }
        case '<':
            if (test("<<")) {
                next(2);
                return std::tuple{line, TOKEN_OP_SHL, "<<"};
            } else if (test("<=")) {
                next(2);
                return std::tuple{line, TOKEN_OP_LE, "<="};
            } else {
                next(1);
                return std::tuple{line, TOKEN_OP_LT, "<"};
            }
        case '>':
            if (test(">>")) {
                next(2);
                return std::tuple{line, TOKEN_OP_SHR, ">>"};
            } else if (test(">=")) {
                next(2);
                return std::tuple{line, TOKEN_OP_GE, ">="};
            } else {
                next(1);
                return std::tuple{line, TOKEN_OP_GT, ">"};
            }
        case '.':
            if (test("...")) {
                next(3);
                return std::tuple{line, TOKEN_VARARG, "..."};
            } else if (test("..")) {
                next(2);
                return std::tuple{line, TOKEN_OP_CONCAT, ".."};
            } else if (chunk->end() - iter == 1 || !std::isdigit(*(iter + 1))) {
                next(1);
                return std::tuple{line, TOKEN_SEP_DOT, "."};
            }
        case '[':
            if (test("[[") || test("[=")) {
                return std::tuple{line, TOKEN_STRING, scanLongString()};
            } else {
                next(1);
                return std::tuple{line, TOKEN_SEP_LBRACK, "["};
            }
        case '\'':
            return std::tuple{line, TOKEN_STRING, scanShortString()};
        case '"':
            return std::tuple{line, TOKEN_STRING, scanShortString()};
    }
    auto c = *iter;
    if (c == '.' || std::isdigit(c)){
        return std::tuple{line, TOKEN_NUMBER, scanNumber()};
    }
    if (c == '_' || isalpha(c)) {
        auto token = scanIdentifier();
        if (keywords.find(token) != keywords.end()){
            return std::tuple{line, keywords[token], token};
        }else{
            return std::tuple{line, TOKEN_IDENTIFIER, token};
        }
    }
    std::cerr << "unexpected symbol near " << c << "line : " << line << std::endl;
    abort();
}

int Lexer::LookAhead(){
    if (nextTokenLine>0){
        return nextTokenKind;
    }
    auto currentLine = line;
    auto [thisLine, kind, token] = NextToken();
    line = currentLine;
    nextTokenLine = thisLine;
    nextTokenKind = kind;
    nextToken = token;
    return kind;
}

std::pair<int, std::string> Lexer::NextTokeOfKind(int kind) {
    auto [thisLine, _kind, token] = NextToken();
    if (kind != _kind){
        std::cerr << "Syntax error near " << token << " line : " << line
        << " kind = " << kind << " _kind = " << _kind << std::endl;
        abort();
    }
    return std::pair{line, token};
}




std::string Lexer::scan(const std::regex & re)  {
    std::regex_search(iter, chunk->cend(), match_result, re);
    if (match_result.empty()){
        std::cerr << "Unreachable!" << std::endl;
        abort();
    }
    next(match_result[0].str().size());
    return match_result[0].str();
}

inline std::string Lexer::scanNumber() {
    return scan(reNumber);
}

inline std::string Lexer::scanIdentifier() {
    return scan(reIdentifier);
}



void Lexer::showLast() const{
    auto iter2 = iter;
    while(iter2 != chunk->end()){
        std::cout << *iter2;
        iter2++;
    }
    std::cout << std::endl;
}