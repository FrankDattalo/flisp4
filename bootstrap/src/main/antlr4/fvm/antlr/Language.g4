grammar Language;

program
    : module import_* export* definition*
    ;

module
    : 'module' identifier ';'
    ;

import_ : 'import' identifier ';'
    ;

export : 'export' identifier ';'
    ;

definition
    : functionDefinition
    ;

functionDefinition
    :   'fn' identifier '(' functionParameters ')' '{' statements? '}'
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
    | tryCatchStatement
    | whileStatement
    | throwStatement
    ;

tryCatchStatement
    : 'try {' statements? '}' 'catch' identifier '{' statements? '}'
    ;

whileStatement
    : 'while' expression '{' statements? '}'
    ;

returnStatement
    : 'return' ';' #emptyReturnStatement
    | 'return' expression ';' #expressionReturnStatement
    ;

throwStatement
    : 'throw' expression ';'
    ;

expressionStatement
    : expression ';'
    ;

expression
    : stringLiteral
    | integerLiteral
    | booleanLiteral
    | nilLiteral
    | variableExpression
    | invokeExpression
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
        '}' #oneArmedIfStatement
    |   'if' expression '{'
            statements?
        '}' 'else' '{'
            statements?
        '}' #twoArmedIfStatement
    ;

invokeExpression
    : identifier '(' functionArguments ')' #internalInvokeExpression
    | identifier ':' identifier '(' functionArguments ')' #externalInvokeExpression
    ;

nilLiteral
    : NIL
    ;

booleanLiteral
    : BOOLEAN
    ;

stringLiteral
    : STRING
    ;

integerLiteral
    : INTEGER
    ;

identifier
    : IDENTIFIER
    ;

INTEGER
    :  ([0-9] | [1-9][0-9]+ )
    ;

NIL
    : 'nil'
    ;

BOOLEAN
    : 'true' | 'false'
    ;

STRING
    : '"' ~([\n"])* '"'
    ;

IDENTIFIER
    : [a-zA-Z_][a-zA-Z_0-9]*
    ;

WS: [\n\t ] -> skip;