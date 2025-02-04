# Grammar for Nav
program = {enumDef | funcDef | structDef};
enumDef = 'enum', IDENTIFIER, '{', IDENTIFIER, {',', IDENTIFIER,} [',',] '}';
funcDef = 'fun', IDENTIFIER, '(', [complexType, IDENTIFIER, {',', complexType, IDENTIFIER,}] ')' [complexType] block;
structDef = 'struct', IDENTIFIER, '{', complexType, IDENTIFIER, {',' complexType, IDENTIFIER}, [','] '}';
complexType = IDENTIFIER | (index | '^', complexType);
block = '{', {statement}, '}';
index = '[', expression, ']';
statement = loneCall | variableDeclaration | ifBlock | forLoop | retStatement | breakStatement | contStatement | switchStatement;
expression = (unaryValue | value), {operator, (unaryValue | value)};
unaryValue = unary, (expression | unaryValue);
value = number | char | string | bracketedValue | makeArray | funcCall | structNew | identifier | access;
access = identifier '.' (access | identifier)
operator = '+' | '-' | '*' | '/' | '&' | '|' | '~';
bracketedValue = '(', expression, ')';
loneCall = funcCall, ';';
makeArray = 'make', '[', [expression, {',', expression}, [',',]] ']';
funcCall = 'call', IDENTIFIER, '(', [expression, {',', expression,} [',',]] ')';
structNew = 'new', IDENTIFIER, '(', [expression, {',', expression,} [',',]] ')';
variableDeclaration -> (assignment | newassignment), ';'
newassignment -> 'let', complexType, IDENTIFIER, '=', expression ;
crement -> '++' | '--', (IDENTIFIER | access), [index];
assignment -> ((IDENTIFIER | access), [index,] '=', expression) | crement;
ifBlock -> 'if', '(', expression, ')', block, ['elif', '(', expression, ')', block,] ['else', block];
forLoop -> 'for', [assignment | newassignment,] ';', [expression,] ';', [assignment,] block;
retStatement -> 'return', [expression], ';';
breakStatement -> 'break', ';';
contStatement -> 'continue', ';';
switchStatement = "switch", "(", expression, ")", "{", {caseBlock}, [defaultBlock], "}";
caseBlock = "case", expression, ":", { statement, } ;
defaultBlock = "default", ":", { statement, } ;
unary = '^' | '--' | '++' | '!' | '`' | '+' | '-' ;
