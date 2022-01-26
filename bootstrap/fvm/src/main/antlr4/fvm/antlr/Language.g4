grammar Language;

program
    : functionDefinition+
    ;

functionDefinition
    : '(' 'define' '(' symbol (symbol)* ')' expression+ ')'
    ;

expression
    : defineExpression
    | setExpression
    | lambdaExpression
    | ifExpression
    | invokeExpression
    | literalExpression
    ;

defineExpression
    : '(' 'define' symbol expression ')'
    ;

setExpression
    : '(' 'set!' symbol expression ')'
    ;

lambdaExpression
    : '(' 'lambda' arguments expression+ ')'
    ;

arguments
    : '(' symbol* ')'
    ;

ifExpression
    : '(' 'if' expression expression expression ')'
    ;

invokeExpression
    : '(' expression expression* ')'
    ;

literalExpression
    : string
    | integer
    | symbol
    ;

string
    : STRING
    ;

integer
    : INTEGER
    ;

symbol
    : IDENTIFIER
    ;

IDENTIFIER
    : [a-zA-Z_][a-zA-Z_0-9]*
    ;

INTEGER
    :  ([0-9] | [1-9][0-9]+ );

STRING
    : '"' ~([\n"])* '"'
    ;

WS: [\n\t ] -> skip;