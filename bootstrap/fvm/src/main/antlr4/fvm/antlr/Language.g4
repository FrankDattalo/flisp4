grammar Language;

program
    : definition+
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
    ;

returnStatement
    : 'return' ';' #emptyReturnStatement
    | 'return' expression ';' #expressionReturnStatement
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

literalExpression
    : stringLiteral
    | integerLiteral
    | booleanLiteral
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