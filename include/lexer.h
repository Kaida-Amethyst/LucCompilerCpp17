//
// Created by ziyue on 4/10/2021.
//

#ifndef CH14_LEXER_H
#define CH14_LEXER_H


#include <string>
#include <memory>
#include <regex>
#include "token.h"

class Lexer{
private:
    std::unique_ptr<std::string> chunk;
    std::string chunkName;
    int line;
    std::string nextToken;
    int nextTokenKind;
    int nextTokenLine;
    std::string::const_iterator iter;
    std::regex reOpeningLongBracket{R"(^\[=*\[)"};
    std::regex reNewLine{R"(\r\n|\n\r|\n|\r)"};
    std::regex reShortStr{R"((^'(\\\\|\\'|\\\n|\\z\s*|[^'\n])*')|(^"(\\\\|\\"|\\\n|\\z\s*|[^"\n])*"))"};
    std::regex reDecEscapeSeq{R"(^\\[0-9]{1,3})"};
    std::regex reHexEscapeSeq{R"(^\\x[0-9a-fA-F]{2})"};
    std::regex reUnicodeEscapeSeq{R"(^\\u\{[0-9a-fA-F]+\})"};
    std::regex reNumber{R"(^0[xX][0-9a-fA-F]*(\.[0-9a-fA-F]*)?([pP][+\-]?[0-9]+)?|^[0-9]*(\.[0-9]*)?([eE][+\-]?[0-9]+)?)"};
    std::regex reIdentifier{R"(^[_\d\w]+)"};
    std::smatch match_result{};


    void skipWhiteSpaces();
    inline void next(int n){ iter += n;};
    inline void next(size_t n){ iter += n;};
    bool test(const std::string & p);
    void skipComment();
    std::string scanLongString();
    std::string scanShortString();
    inline std::string scanNumber();
    std::string scan(const std::regex & re) ;
    inline std::string scanIdentifier();

    std::string escape(const std::string & full_str);
    static inline bool isNewLine(const char & c) ;
    static inline bool isWhiteSpace(const char & c) ;

public:
    Lexer(std::string chunkName, const std::string & chunk);
    Lexer(Lexer & ) = delete;
    std::tuple<int, int, std::string> NextToken();
    std::pair<int, std::string> NextTokeOfKind(int kind);
    void showLast() const;

    inline int Line() const {return line;}

    int LookAhead();

    inline std::pair<int, std::string> NextIdentifier() {return NextTokeOfKind(TOKEN_IDENTIFIER);}
};

#endif //CH14_LEXER_H
