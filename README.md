# PL0设计
## 编译器结构
PLO编译器的结构如下：
- 全局变量：控制整个编译过程，同时存储一些指令/符号集合
- 错误处理
- 读取符号
- 语句块处理器
- 解释器
## 接口设计
- `error`: 输出错误
- `getsym`: 读取下一个token
- `gen`: 生成一条编译出的目标代码
- `test`: 检查能否访问符号
- `block`: 处理一个代码块
  - `enter`: 将变量/常量写入符号表
  - `position`: 根据标识符查找符号表表索引
  - `constdeclaration`: 处理常量声明
  - `vardeclaration`: 处理变量生命
  - `listcode`: 输出目前已经编译出的目标代码
  - `statement`: 处理语句
    - `expression`: 处理表达式
    - `term`: 处理项
      - `factor`: 处理因子
    - `condition`: 处理条件表达式
- `interpret`: 解释执行编译出的程序
## 文件结构
pl0编译器仅有一个文件组成。
由main函数调用子程序控制编译过程进行。
编译过程为block函数。
解释过程为interpret函数。
# 编译器结构
- `Lexer`: 词法分析
- `Compiler`: 一次编译过程
# 文件组织
- `lexer`: 词法分析相关
- `CMakeLists.txt`: 编译文件，并通过`add_definitions`来控制debug输出与每次作业的输出
- `core`: 核心部分，例如编译过程的抽象
- `main.cpp`: 程序入口
# 接口设计
## Token
Token相关的设计有TokenType和Token,TokenType结合X-Macro实现了enum到string的快速转换； Token存储了TokenType,原内容和行数。
- `Token()`: 创建空的Token
- `Token(TokenType, std::string const&, int)`:创建初始化的Token
- `tokenTypeToString`:将TokenType转换为字符串值。
- `reserve`:使用了哈希表实现关键字查询。
## Lexer
词法分析器
- `Lexer(std::istream&)`:使用输入流创建词法分析器
- `next(Token&)`:获取字符流中的下一个token，这部分使用了有限状态机来实现。

## Compiler
一次编译过程
- `Compiler(std::string const&)`:创建一个编译过程
- `lexer()`:进行词法分析，存储解析的token
- `printTokens()`:输出解析的token,用于第一次作业

#  词法分析

编码前设计：通过某个方法对字符流进行逐字符处理，使用有限状态机来解析token，最后存储到token表中。同时，单独存储错误的token, 方便后续输出

编码完成之后的修改：通过调用`Compiler::lexer`来完成一次编译过程中的词法分析，不断调用词法分析器`Lexer::next`并记录解析出来的token，存储到变量`tokens`中，如果是错误的token(如`&`,`|`)，还会存储到errorTokens中。最后，如果由宏定义PRINT_LEXER，则会调用`Compiler::printTokens`按照第一次实验的输出格式输出到`lexer.txt`和`error.txt`