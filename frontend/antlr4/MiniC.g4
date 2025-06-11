grammar MiniC;

// 词法规则名总是以大写字母开头

// 语法规则名总是以小写字母开头

// 每个非终结符尽量多包含闭包、正闭包或可选符等的EBNF范式描述

// 若非终结符由多个产生式组成，则建议在每个产生式的尾部追加# 名称来区分，详细可查看非终结符statement的描述

// 语法规则描述：EBNF范式

// 源文件编译单元定义
compileUnit: (funcDef | varDecl)* EOF;

// 函数定义，支持形参，支持返回int和void类型
funcDef:
	funcReturnType T_ID T_L_PAREN formalParamList? T_R_PAREN block;

// 函数返回类型
funcReturnType: T_INT | T_VOID;

// 形参列表
formalParamList: formalParam (T_COMMA formalParam)*;

// 形参定义
formalParam: T_INT T_ID;

// 语句块看用作函数体，这里允许多个语句，并且不含任何语句
block: T_L_BRACE blockItemList? T_R_BRACE;

// 每个ItemList可包含至少一个Item
blockItemList: blockItem+;

// 每个Item可以是一个语句，或者变量声明语句
blockItem: statement | varDecl;

// 变量声明，支持变量初始化
varDecl: T_INT varDef (T_COMMA varDef)* T_SEMICOLON;

// 基本类型（用于函数返回类型）
basicType: T_INT | T_VOID;

// 变量定义，支持可选的初始化
varDef: T_ID (T_ASSIGN expr)?;

// 语句支持多种形式，添加了if语句、while语句、break和continue语句
statement:
	T_RETURN expr? T_SEMICOLON										# returnStatement
	| lVal T_ASSIGN expr T_SEMICOLON								# assignStatement
	| block															# blockStatement
	| T_IF T_L_PAREN expr T_R_PAREN statement (T_ELSE statement)?	# ifStatement
	| T_WHILE T_L_PAREN expr T_R_PAREN statement					# whileStatement
	| T_BREAK T_SEMICOLON											# breakStatement
	| T_CONTINUE T_SEMICOLON										# continueStatement
	| expr? T_SEMICOLON												# expressionStatement;

// 表达式文法: 最低优先级是逻辑或表达式
expr: logicalOrExp;

// 逻辑或表达式
logicalOrExp: logicalAndExp (T_OR logicalAndExp)*;

// 逻辑与表达式
logicalAndExp: equalityExp (T_AND equalityExp)*;

// 相等比较表达式
equalityExp: relationalExp (equalityOp relationalExp)*;
equalityOp: T_EQ | T_NE;

// 关系比较表达式
relationalExp: addExp (relationalOp addExp)*;
relationalOp: T_LT | T_GT | T_LE | T_GE;

// 加减表达式（最低优先级）
addExp: mulExp (addOp mulExp)*;
addOp: T_ADD | T_SUB;

// 乘除取模表达式（中等优先级）
mulExp: unaryExp (mulOp unaryExp)*;
mulOp: T_MUL | T_DIV | T_MOD;

// 一元表达式（最高优先级）
unaryExp:
	T_SUB unaryExp
	| T_NOT unaryExp
	| primaryExp
	| T_ID T_L_PAREN realParamList? T_R_PAREN;

// 基本表达式：括号表达式、整数、左值表达式
primaryExp:
	T_L_PAREN expr T_R_PAREN
	| T_DIGIT
	| T_DIGIT_LL
	| lVal;

// 实参列表
realParamList: expr (T_COMMA expr)*;

// 左值表达式
lVal: T_ID;

// 用正规式来进行词法规则的描述

T_L_PAREN: '(';
T_R_PAREN: ')';
T_SEMICOLON: ';';
T_L_BRACE: '{';
T_R_BRACE: '}';

T_ASSIGN: '=';
T_COMMA: ',';

T_ADD: '+';
T_SUB: '-';
T_MUL: '*';
T_DIV: '/';
T_MOD: '%';

// 关系运算符
T_LT: '<';
T_GT: '>';
T_LE: '<=';
T_GE: '>=';
T_EQ: '==';
T_NE: '!=';

// 逻辑运算符
T_AND: '&&';
T_OR: '||';
T_NOT: '!';

// 控制语句关键字
T_IF: 'if';
T_ELSE: 'else';
T_WHILE: 'while';
T_BREAK: 'break';
T_CONTINUE: 'continue';

// 要注意关键字同样也属于T_ID，因此必须放在T_ID的前面，否则会识别成T_ID
T_RETURN: 'return';
T_INT: 'int';
T_VOID: 'void';

T_ID: [a-zA-Z_][a-zA-Z0-9_]*;
T_DIGIT_LL:
	'0' [xX] [0-9a-fA-F]+ [lL]
	| '0' [0-7]+ [lL]
	| [1-9][0-9]* [lL];
T_DIGIT:
	'0' [xX] [0-9a-fA-F]+
	| '0' [0-7]+
	| '0'
	| [1-9][0-9]*;

// 单行注释
LINE_COMMENT: '//' ~[\r\n]* -> skip;

/* 空白符丢弃 */
WS: [ \r\n\t]+ -> skip;