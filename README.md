# 编译器
- 词法分析
# 项目结构
- `lexer`: 词法分析相关
- `CMakeLists.txt`: 编译文件，并通过`add_definitions`来控制debug输出与每次作业的输出
- `core`: 核心部分，例如编译过程的抽象
- `main.cpp`: 程序入口
# 模块
## Token
Token相关的设计有TokenType和Token,TokenType结合X-Macro实现了enum到string的快速转换； Token存储了TokenType,原内容和行数。

- `tokenTypeToString`:将TokenType转换为字符串值。
- `reserve`:使用了哈希表实现关键字查询。
## Lexer
词法分析器
- `next(Token&)`:获取字符流中的下一个token，这部分使用了有限状态机来实现。

## Compiler
一次编译过程
- `Compiler(std::string const&)`:创建一个编译过程
- `lexer`:进行词法分析，存储解析的token
- `printTokens`:输出解析的token,用于第一次作业