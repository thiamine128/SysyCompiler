# Lexer
## Token
Token相关的设计有TokenType和Token,TokenType结合X-Macro实现了enum到string的快速转换； Token存储了TokenType,原内容和行数。

- `tokenTypeToString`:将TokenType转换为字符串值。
- `reserve`:使用了哈希表实现关键字查询。
## Lexer
词法分析器
- `next(Token&)`:获取字符流中的下一个token，这部分使用了有限状态机来实现。
