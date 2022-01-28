grammar Language;

program
    : definition+
    ;

definition
    : functionDefinition
    ;

functionDefinition
    :   'fn' identifier '(' functionParameters ')' '{' statements '}'
    ;

functionParameters
    :
    | identifier (',' identifier)*
    ;

functionArguments
    :
    | expression (',' expression)*
    ;

statements
    : statement+
    ;

statement
    : ifStatement
    | assignStatement
    | defineStatement
    | expressionStatement
    | returnStatement
    ;

returnStatement
    : 'return' ';'
    | 'return' expression ';'
    ;

expressionStatement
    : expression ';'
    ;

expression
    : invokeExpression
    | literalExpression
    | variableExpression
    ;

variableExpression
    : identifier
    ;

defineStatement
    : 'var' identifier '=' expression ';'
    ;

assignStatement
    : identifier '=' expression ';'
    ;

ifStatement
    :   'if' expression '{' 
            statements?
        '}'
    |   'if' expression '{'
            statements?
        '}' 'else' '{'
            statements?
        '}'
    ;

invokeExpression
    : identifier '(' functionArguments ')'
    | identifier ':' identifier '(' functionArguments ')'
    ;

literalExpression
    : string
    | integer
    | identifier
    | bool
    ;

bool
    : BOOLEAN
    ;

string
    : STRING
    ;

integer
    : INTEGER
    ;

identifier
    : IDENTIFIER
    ;

IDENTIFIER
    : [a-zA-Z_][a-zA-Z_0-9]*
    ;

INTEGER
    :  ([0-9] | [1-9][0-9]+ )
    ;

BOOLEAN
    : 'true' | 'false'
    ;

STRING
    : '"' ~([\n"])* '"'
    ;

WS: [\n\t ] -> skip;