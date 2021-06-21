# LucCompilerCpp17

自己用C++17写的lua编译器，目前已经完成了词法分析，语法分析，字节码生成以及解析的部分。

目前此编译器还不够完善。一是缺乏大量标准库，二是仍然存在许多暗病。未来会逐步修复这些暗病，并且会进一步完善文档。

## 查看文档

查阅documents文件夹下。（文档暂时不全，未来会慢慢补全）

## 运行lua代码

目前的办法（暂时）：需要修改main函数

```c++
    std::ifstream file;
    file.open("../luafiles/hello_world.lua", std::ios::in);
    std::istreambuf_iterator<char> beg(file), end;
    std::string st{beg, end};
```

填入lua代码文件的位置即可。

## 编译

使用cmake即可。