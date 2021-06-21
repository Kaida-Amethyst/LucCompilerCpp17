# 词法分析器 Lexer

## 构造函数

```c++
Lexer(std::string chunkName, const std::string & chunk);
```

传入块代码的名称和代码字符串。

## 成员变量

```c++
std::unique_ptr<std::string> chunk;   // 代码字符串指针，这个指针是始终不动的
std::string chunkName;                // 块代码名称
int line;                             // 分析到的行号
std::string::const_iterator iter;     // 块代码字符串迭代器，用于分析代码
```

## private函数

### next函数

```c++
inline void next(int n){ iter += n;};
```

将代码字符串向后移动n位。

### test函数

```c++
bool test(const std::string & p);
```

测试剩下的代码是否以字符串p开头，常用的场景是lua代码中以"--"开头代表注释，所以需要设计test函数来判断iter指向剩下的代码部分是否以"--"开头。

### skipWhiteSpace函数

```c++
void Lexer::skipWhiteSpaces()
```

跳过空白空格和注释，如果iter指向的内容确实是空格，那么就会把iter往下移动。如果iter指向的内容是注释，那么会把iter往后移动到注释的末尾。并且会适时地更新行号。

### skipComment函数

```c++
void Lexer::skipComment()
```

用于在分析时跳过注释。

### scan函数

```c++
std::string Lexer::scan(const std::regex & re)
```

scan函数用于从iter开始之后，扫描符合给定正则表达式的字符串，并且推进iter。

### scanNumber函数

```c++
inline std::string scanNumber();
```

scanNumber用于读入数字，注意返回的仍是字符串。

### scanLongString函数

```c++
std::string Lexer::scanLongString()
```

除了一般的用引号括起，lua的字符串有一种特殊的表达方式是用两个方括号，更特殊的用法是是左右对称的两个双方括号，两个同向的方括号里面有相同的=号。

```lua
str = [[abcd]]
str2 = [==[abcd
ppp]==]
```

这个函数用于提取方括号中的内容，譬如提取上面str的内容，就是abcd。

### scanShortString函数

```c++
std::string Lexer::scanShortString()
```

短字符串使用单引号或者双引号括起来的字符串，这个函数用于提取短字符串引号括起来的部分。

### escape函数

escape函数用于处理字符串中的转义字符串，如将"\n"（斜杠+字符n）处理成换行符。注意lua中的\ddd，\x, \u尚未经过测试

```c++
std::string Lexer::escape()
```

### LookAhead函数

```
LookAhead()
```

LookAhead函数用于在分析中看一下缓存下一个token的信息。

### NextTokenOfKind函数

```c++
std::pair<int, std::string> Lexer::NextTokeOfKind(int kind)
```

NextTokenOfKind函数用于提取下一个token，但是要求下一个token必须是指定类型的。

## 静态函数

### isWhiteSpace, isNewLine

```c++
inline bool Lexer::isWhiteSpace(const char & c) {
    return c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' ';
}

inline bool Lexer::isNewLine(const char & c) {
    return c == '\r' || c == '\n';
}
```

isWhiteSpace用于判断某个字符是否是空白字符。

isNewLine用于判断代码是否到了下一行。