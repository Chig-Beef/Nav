#include "Node.h"
#include "Panic.h"
#include "String.h"
#include "Token.h"
#include <stdio.h>

#define CHECK_TOK(tokenCode, expected)                                         \
  if (p->tok.kind != (tokenCode)) {                                            \
    throwParserError(p, (expected));                                           \
  }

#define APPEND_NODE(nodeCode, nodeData, funcName)                              \
  if (NodeListAppend(&out.children,                                            \
                     newNode((nodeCode), (nodeData), p->tok.line))) {          \
    panic("Couldn't append to Node list in " funcName);                        \
  }

#define CHECK_AND_APPEND(tokenCode, expected, nodeCode, nodeData, funcName)    \
  CHECK_TOK((tokenCode), (expected))                                           \
  APPEND_NODE((nodeCode), (nodeData), funcName)

#define CHECK_APPEND_NEXT(tokenCode, expected, nodeCode, nodeData, funcName)   \
  CHECK_TOK((tokenCode), (expected))                                           \
  APPEND_NODE((nodeCode), (nodeData), funcName)                                \
  nextToken(p);

#define APPEND_STRUCTURE(func, funcName)                                       \
  if (NodeListAppend(&out.children, func(p))) {                                \
    panic("Couldn't append to Node list in " funcName);                        \
  }

typedef struct Parser {
  char *sourceName;
  TokenList source;
  Token tok;
  int index;
  Node out;
} Parser;

void parserInit(Parser *p, char *sourceName, TokenList source) {
  p->sourceName = sourceName;
  p->source = source;
  p->index = 0;
  p->tok.kind = T_ILLEGAL;
}

// Because the AST is a tree structure, we should pre-define everything
Node parseStruct(Parser *p);
Node parseEnum(Parser *p);
Node parseFunc(Parser *p);
Node parseOperator(Parser *p);
Node parseIndex(Parser *p);
Node parseIfBlock(Parser *p);
Node parseForLoop(Parser *p);
Node parseRetState(Parser *p);
Node parseBreakState(Parser *p);
Node parseContinueState(Parser *p);
Node parseBracketedValue(Parser *p);
Node parseStructNew(Parser *p);
Node parseFuncCall(Parser *p);
Node parseMakeArray(Parser *p);
Node parseLoneCall(Parser *p);
Node parseExpression(Parser *p);
Node parseCrement(Parser *p);
Node parseAssignment(Parser *p);
Node parseNewAssignment(Parser *p);
Node parseVarDeclaration(Parser *p);
Node parseUnary(Parser *p);
Node parseUnaryValue(Parser *p);
Node parseComplexType(Parser *p);
Node parseValue(Parser *p);
Node parseSwitchState(Parser *p);
Node parseCaseBlock(Parser *p);
Node parseDefaultBlock(Parser *p);
Node parseBlock(Parser *p);
void parse(Parser *p);

void nextToken(Parser *p) {
  Token t = {NULL, T_ILLEGAL, 0};

  if (p->index < p->source.len) {
    t = p->source.p[p->index];
  }

  ++p->index;
  p->tok = t;
}

void prevToken(Parser *p) {
  Token t = {NULL, T_ILLEGAL, 0};

  if (p->index > 0) {
    t = p->source.p[p->index - 1];
  }

  --p->index;
  p->tok = t;
}

Token peekToken(Parser *p) {
  if (p->index < p->source.len) {
    return p->source.p[p->index];
  }

  return (Token){NULL, T_ILLEGAL, 0};
}

void throwParserError(Parser *p, char expected[]) {
  printf("Error in the Parser!\n"
         "Error found in file: %s\nOn line: %i\nExpected: %s\nGot: %s\n",
         p->sourceName, p->tok.line, expected, tokenString(p->tok));
  exit(1);
}

Node parseStruct(Parser *p) {
  Node out = newNode(N_STRUCT_DEF, strNew("Struct Def", false), p->tok.line);

  CHECK_APPEND_NEXT(T_STRUCT, "struct", N_STRUCT, NULL, "parseStruct")
  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseStruct")
  CHECK_APPEND_NEXT(T_L_SQUIRLY, "{", N_L_SQUIRLY, NULL, "parseStruct")

  APPEND_STRUCTURE(parseComplexType, "parseStruct");
  nextToken(p);

  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseStruct")

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseStruct")
    nextToken(p);

    APPEND_STRUCTURE(parseComplexType, "parseStruct");
    nextToken(p);

    CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                      strGet(p->tok.data), "parseStruct")
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseStruct")
    nextToken(p);
  }

  CHECK_APPEND_NEXT(T_R_SQUIRLY, "}", N_R_SQUIRLY, NULL, "parseStruct")

  return out;
}

Node parseEnum(Parser *p) {
  Node out = newNode(N_ENUM_DEF, strNew("Enum Def", false), p->tok.line);

  CHECK_APPEND_NEXT(T_ENUM, "enum", N_ENUM, NULL, "parseEnum")
  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseEnum")
  CHECK_APPEND_NEXT(T_L_SQUIRLY, "{", N_L_SQUIRLY, NULL, "parseEnum")
  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseEnum")

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseEnum")
    nextToken(p);

    CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                      strGet(p->tok.data), "parseEnum")
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseEnum")
    nextToken(p);
  }

  CHECK_APPEND_NEXT(T_R_SQUIRLY, "}", N_R_SQUIRLY, NULL, "parseEnum")

  return out;
}

Node parseFunc(Parser *p) {
  Node out = newNode(N_FUNC_DEF, strNew("Func Def", false), p->tok.line);

  CHECK_APPEND_NEXT(T_FUN, "fun", N_FUN, NULL, "parseFunc")
  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseFunc")
  CHECK_APPEND_NEXT(T_L_PAREN, "(", N_L_PAREN, NULL, "parseFunc")

  // We have params
  if (p->tok.kind != T_R_PAREN) {
    APPEND_STRUCTURE(parseComplexType, "parseFunc");
    nextToken(p);

    CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                      strGet(p->tok.data), "parseFunc")

    while (p->tok.kind == T_SEP) {
      APPEND_NODE(N_SEP, NULL, "parseFunc")
      nextToken(p);

      APPEND_STRUCTURE(parseComplexType, "parseFunc");
      nextToken(p);

      CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                        strGet(p->tok.data), "parseFunc")
    }
  }

  CHECK_APPEND_NEXT(T_R_PAREN, ")", N_R_PAREN, NULL, "parseFunc")

  // Return type
  if (p->tok.kind != T_L_SQUIRLY) {
    APPEND_STRUCTURE(parseComplexType, "parseFunc");
    nextToken(p);
  }

  APPEND_STRUCTURE(parseBlock, "parseFunc");

  return out;
}

Node parseOperator(Parser *p) {
  switch (p->tok.kind) {
  case T_ADD:
    return newNode(N_ADD, NULL, p->tok.line);
  case T_AND:
    return newNode(N_AND, NULL, p->tok.line);
  case T_ANDAND:
    return newNode(N_ANDAND, NULL, p->tok.line);
  case T_DIV:
    return newNode(N_DIV, NULL, p->tok.line);
  case T_EQ:
    return newNode(N_EQ, NULL, p->tok.line);
  case T_GT:
    return newNode(N_GT, NULL, p->tok.line);
  case T_GTEQ:
    return newNode(N_GTEQ, NULL, p->tok.line);
  case T_LT:
    return newNode(N_LT, NULL, p->tok.line);
  case T_LTEQ:
    return newNode(N_LTEQ, NULL, p->tok.line);
  case T_MOD:
    return newNode(N_MOD, NULL, p->tok.line);
  case T_MUL:
    return newNode(N_MUL, NULL, p->tok.line);
  case T_NEQ:
    return newNode(N_NEQ, NULL, p->tok.line);
  case T_OR:
    return newNode(N_OR, NULL, p->tok.line);
  case T_OROR:
    return newNode(N_OROR, NULL, p->tok.line);
  case T_SUB:
    return newNode(N_SUB, NULL, p->tok.line);
  case T_XOR:
    return newNode(N_XOR, NULL, p->tok.line);
  case T_L_SHIFT:
    return newNode(N_L_SHIFT, NULL, p->tok.line);
  case T_R_SHIFT:
    return newNode(N_R_SHIFT, NULL, p->tok.line);

  default:
    // Don't error, let caller handle it
    return ZERO_NODE;
  }
}

Node parseIndex(Parser *p) {
  Node out = newNode(N_INDEX, strNew("Index", false), p->tok.line);

  CHECK_APPEND_NEXT(T_L_BLOCK, "[", N_L_BLOCK, NULL, "parseIndex")

  APPEND_STRUCTURE(parseExpression, "parseIndex");
  nextToken(p);

  CHECK_AND_APPEND(T_R_BLOCK, "]", N_R_BLOCK, NULL, "parseIndex")

  return out;
}

Node parseIfBlock(Parser *p) {
  Node out = newNode(N_IF_BLOCK, strNew("If Block", false), p->tok.line);

  CHECK_APPEND_NEXT(T_IF, "if", N_IF, NULL, "parseIfBlock")
  CHECK_APPEND_NEXT(T_L_PAREN, "(", N_L_PAREN, NULL, "parseIfBlock")

  APPEND_STRUCTURE(parseExpression, "parseIfBlock");
  nextToken(p);

  CHECK_APPEND_NEXT(T_R_PAREN, ")", N_R_PAREN, NULL, "parseIfBlock")
  APPEND_STRUCTURE(parseBlock, "parseIfBlock");

  if (peekToken(p).kind == T_ELIF) {
    nextToken(p);
    APPEND_NODE(N_ELIF, NULL, "parseIfBlock")
    nextToken(p);

    CHECK_APPEND_NEXT(T_L_PAREN, "(", N_L_PAREN, NULL, "parseIfBlock")

    APPEND_STRUCTURE(parseExpression, "parseIfBlock")
    nextToken(p);

    CHECK_APPEND_NEXT(T_R_PAREN, ")", N_R_PAREN, NULL, "parseIfBlock")
    APPEND_STRUCTURE(parseBlock, "parseIfBlock")
  }

  if (peekToken(p).kind == T_ELSE) {
    nextToken(p);
    APPEND_NODE(N_ELSE, NULL, "parseIfBlock")
    nextToken(p);

    APPEND_STRUCTURE(parseBlock, "parseIfBlock")
  }

  return out;
}

Node parseForLoop(Parser *p) {
  Node out = newNode(N_FOR_LOOP, strNew("For Loop", false), p->tok.line);

  CHECK_APPEND_NEXT(T_FOR, "for", N_FOR, NULL, "parseForLoop")
  CHECK_APPEND_NEXT(T_L_PAREN, "(", N_L_PAREN, NULL, "parseForLoop")

  if (p->tok.kind == T_LET) {
    APPEND_STRUCTURE(parseNewAssignment, "parseForLoop")
    nextToken(p);
  } else if (p->tok.kind != T_SEMICOLON) {
    APPEND_STRUCTURE(parseAssignment, "parseForLoop")
    nextToken(p);
  }

  CHECK_APPEND_NEXT(T_SEMICOLON, ";", N_SEMICOLON, NULL, "parseForLoop")

  if (p->tok.kind != T_SEMICOLON) {
    APPEND_STRUCTURE(parseExpression, "parseForLoop")
    nextToken(p);
  }

  CHECK_APPEND_NEXT(T_SEMICOLON, ";", N_SEMICOLON, NULL, "parseForLoop")

  if (p->tok.kind != T_R_PAREN) {
    APPEND_STRUCTURE(parseAssignment, "parseForLoop")
    nextToken(p);
  }

  CHECK_APPEND_NEXT(T_R_PAREN, ")", N_R_PAREN, NULL, "parseForLoop")
  APPEND_STRUCTURE(parseBlock, "parseForLoop")

  return out;
}

Node parseRetState(Parser *p) {
  Node out = newNode(N_RET_STATE, strNew("Ret State", false), p->tok.line);

  CHECK_APPEND_NEXT(T_RETURN, "return", N_RETURN, NULL, "parseRetState")

  if (p->tok.kind != ';') {
    APPEND_STRUCTURE(parseExpression, "parseRetState")
    nextToken(p);
  }

  CHECK_AND_APPEND(T_SEMICOLON, ";", N_SEMICOLON, NULL, "parseRetState")

  return out;
}

Node parseBreakState(Parser *p) {
  Node out = newNode(N_BREAK_STATE, strNew("Break State", false), p->tok.line);

  CHECK_APPEND_NEXT(T_BREAK, "break", N_BREAK, NULL, "parseBreakState")
  CHECK_AND_APPEND(T_SEMICOLON, ";", N_SEMICOLON, NULL, "parseBreakState")

  return out;
}

Node parseContinueState(Parser *p) {
  Node out =
      newNode(N_CONTINUE_STATE, strNew("Continue State", false), p->tok.line);

  CHECK_APPEND_NEXT(T_CONTINUE, "continue", N_CONTINUE, NULL,
                    "parseContinueState")
  CHECK_AND_APPEND(T_SEMICOLON, ";", N_SEMICOLON, NULL, "parseContinueState")

  return out;
}

Node parseBracketedValue(Parser *p) {
  Node out =
      newNode(N_BRACKETED_VALUE, strNew("Bracketed Value", false), p->tok.line);

  CHECK_APPEND_NEXT(T_L_PAREN, "(", N_L_PAREN, NULL, "parseBracketedValue")

  APPEND_STRUCTURE(parseExpression, "parseBracketedValue")
  nextToken(p);

  CHECK_AND_APPEND(T_R_PAREN, ")", N_R_PAREN, NULL, "parseBracketedValue")

  return out;
}

Node parseStructNew(Parser *p) {
  Node out = newNode(N_STRUCT_NEW, strNew("Struct New", false), p->tok.line);

  CHECK_APPEND_NEXT(T_STRUCT, "struct", N_STRUCT, NULL, "parseStructNew")
  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseStructNew")

  CHECK_APPEND_NEXT(T_L_PAREN, "(", N_L_PAREN, NULL, "parseStructNew")

  APPEND_STRUCTURE(parseExpression, "parseStructNew")
  nextToken(p);

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseStructNew")
    nextToken(p);

    APPEND_STRUCTURE(parseExpression, "parseStructNew")
    nextToken(p);
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseStructNew")
    nextToken(p);
  }

  CHECK_AND_APPEND(T_R_PAREN, ")", N_R_PAREN, NULL, "parseStructNew")

  return out;
}

Node parseFuncCall(Parser *p) {
  Node out = newNode(N_FUNC_CALL, strNew("Func Call", false), p->tok.line);

  CHECK_APPEND_NEXT(T_FUN, "fun", N_FUN, NULL, "parseFuncCall")
  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseFuncCall")
  CHECK_APPEND_NEXT(T_L_PAREN, "(", N_L_PAREN, NULL, "parseFuncCall")

  APPEND_STRUCTURE(parseExpression, "parseFuncCall")
  nextToken(p);

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseFuncCall")
    nextToken(p);

    APPEND_STRUCTURE(parseExpression, "parseFuncCall")
    nextToken(p);
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseFuncCall")
    nextToken(p);
  }

  CHECK_AND_APPEND(T_R_PAREN, ")", N_R_PAREN, NULL, "parseFuncCall")

  return out;
}

Node parseMakeArray(Parser *p) {
  Node out = newNode(N_MAKE_ARRAY, strNew("Make Array", false), p->tok.line);

  CHECK_APPEND_NEXT(T_MAKE, "make", N_MAKE, NULL, "parseMakeArray")
  CHECK_APPEND_NEXT(T_L_BLOCK, "[", N_L_BLOCK, NULL, "parseMakeArray")

  APPEND_STRUCTURE(parseExpression, "parseMakeArray")
  nextToken(p);

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseMakeArray")
    nextToken(p);

    APPEND_STRUCTURE(parseExpression, "parseMakeArray")
    nextToken(p);
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseMakeArray")
    nextToken(p);
  }

  CHECK_APPEND_NEXT(T_R_BLOCK, "]", N_R_BLOCK, NULL, "parseMakeArray")

  return out;
}

Node parseLoneCall(Parser *p) {
  Node out = newNode(N_LONE_CALL, strNew("Lone Call", false), p->tok.line);

  APPEND_STRUCTURE(parseFuncCall, "parseLoneCall");
  nextToken(p);

  CHECK_AND_APPEND(T_SEMICOLON, ";", N_SEMICOLON, NULL, "parseLoneCall")

  return out;
}

Node parseExpression(Parser *p) {
  Node out = newNode(N_EXPRESSION, strNew("Expression", false), p->tok.line);

  if (parseUnary(p).kind == N_ILLEGAL) {
    Node n = parseValue(p);
    if (n.kind == N_ILLEGAL) {
      throwParserError(p, "value");
    }
    if (NodeListAppend(&out.children, n)) {
      panic("Couldn't append to Node list in parseExpression");
    }
  } else {
    APPEND_STRUCTURE(parseUnaryValue, "parseExpression");
  }
  nextToken(p);

  Node n = parseOperator(p);
  while (n.kind != N_ILLEGAL) {
    if (NodeListAppend(&out.children, n)) {
      panic("Couldn't append to Node list in parseExpression");
    }
    nextToken(p);

    if (parseUnary(p).kind == N_ILLEGAL) {
      n = parseValue(p);
      if (n.kind == N_ILLEGAL) {
        throwParserError(p, "value");
      }
      if (NodeListAppend(&out.children, n)) {
        panic("Couldn't append to Node list in parseExpression");
      }
    } else {
      APPEND_STRUCTURE(parseUnaryValue, "parseExpression");
    }
    nextToken(p);

    n = parseOperator(p);
  }

  prevToken(p);

  return out;
}

Node parseCrement(Parser *p) {
  Node out = newNode(N_CREMENT, strNew("Crement", false), p->tok.line);

  if (p->tok.kind == T_INC) {
    APPEND_NODE(N_INC, NULL, "parseCrement")
  } else if (p->tok.kind == T_DEC) {
    APPEND_NODE(N_DEC, NULL, "parseCrement")
  } else {
    throwParserError(p, "inc or dec");
  }
  nextToken(p);

  CHECK_AND_APPEND(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                   strGet(p->tok.data), "parseCrement")

  return out;
}

Node parseAssignment(Parser *p) {
  Node out = newNode(N_ASSIGNMENT, strNew("Assignment", false), p->tok.line);

  // An assignment can just be a crement
  if (p->tok.kind == T_INC || p->tok.kind == T_DEC) {
    APPEND_STRUCTURE(parseCrement, "parseAssignment");
    return out;
  }

  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseAssignment")

  // Assigning to element in array?
  if (p->tok.kind == T_L_BLOCK) {
    APPEND_STRUCTURE(parseIndex, "parseAssignment");
    nextToken(p);
  }

  CHECK_APPEND_NEXT(T_ASSIGN, "=", N_ASSIGN, NULL, "parseAssignment")
  APPEND_STRUCTURE(parseExpression, "parseAssignment");

  return out;
}

Node parseNewAssignment(Parser *p) {
  Node out =
      newNode(N_NEW_ASSIGNMENT, strNew("New Assignment", false), p->tok.line);

  CHECK_APPEND_NEXT(T_LET, "let", N_LET, NULL, "parseNewAssignment")

  APPEND_STRUCTURE(parseComplexType, "parseNewAssignment");
  nextToken(p);

  CHECK_APPEND_NEXT(T_IDENTIFIER, "identifier", N_IDENTIFIER,
                    strGet(p->tok.data), "parseNewAssignment")
  CHECK_APPEND_NEXT(T_ASSIGN, "=", N_ASSIGN, NULL, "parseNewAssignment")
  APPEND_STRUCTURE(parseExpression, "parseNewAssignment");

  return out;
}

Node parseVarDeclaration(Parser *p) {
  Node out = newNode(N_VAR_DEC, strNew("Var Declaration", false), p->tok.line);

  // New variable
  if (p->tok.kind == T_LET) {
    APPEND_STRUCTURE(parseNewAssignment, "parseVarDeclaration");

    // Old variable
  } else {
    APPEND_STRUCTURE(parseAssignment, "parseVarDeclaration");
  }

  nextToken(p);

  CHECK_AND_APPEND(T_SEMICOLON, ";", N_SEMICOLON, NULL, "parseVarDeclaration")

  return out;
}

Node parseUnary(Parser *p) {
  switch (p->tok.kind) {
  case T_DEREF:
    return newNode(N_DEREF, NULL, p->tok.line);
  case T_DEC:
    return newNode(N_DEC, NULL, p->tok.line);
  case T_INC:
    return newNode(N_INC, NULL, p->tok.line);
  case T_NOT:
    return newNode(N_NOT, NULL, p->tok.line);
  case T_REF:
    return newNode(N_REF, NULL, p->tok.line);
  case T_ADD:
    return newNode(N_ADD, NULL, p->tok.line);
  case T_SUB:
    return newNode(N_SUB, NULL, p->tok.line);

  default:
    // Don't error, let caller handle it
    return ZERO_NODE;
  }
}

Node parseUnaryValue(Parser *p) {
  Node out = newNode(N_UNARY_VALUE, strNew("Unary Value", false), p->tok.line);

  Node n = parseUnary(p);

  if (n.kind == N_ILLEGAL) {
    throwParserError(p, "unary");
  }

  if (NodeListAppend(&out.children, n)) {
    panic("Couldn't append to Node list in parseUnaryValue");
  }
  nextToken(p);

  if (parseUnary(p).kind != N_ILLEGAL) {
    APPEND_STRUCTURE(parseUnaryValue, "parseUnaryValue");
  } else {
    n = parseValue(p);
    if (n.kind == N_ILLEGAL) {
      throwParserError(p, "value");
    }
    if (NodeListAppend(&out.children, n)) {
      panic("Couldn't append to Node list in parseUnaryValue");
    }
  }

  return out;
}

Node parseComplexType(Parser *p) {
  Node out =
      newNode(N_COMPLEX_TYPE, strNew("Complex Type", false), p->tok.line);

  // Type is only one word
  if (p->tok.kind == ((T_IDENTIFIER))) {
    APPEND_NODE(N_IDENTIFIER, strGet(p->tok.data), "parseComplexType")
    return out;
  }

  if (p->tok.kind == T_L_BLOCK) { // Index
    APPEND_STRUCTURE(parseIndex, "parseComplexType");
  } else if (p->tok.kind == T_DEREF) { // Deref
    APPEND_NODE(N_DEREF, NULL, "parseComplexType")
  } else { // Bad token in type
    throwParserError(p, "index or deref");
  }

  nextToken(p);

  APPEND_STRUCTURE(parseComplexType, "parseComplexType");

  return out;
}

Node parseValue(Parser *p) {
  switch (p->tok.kind) {
  case T_INT:
    return newNode(N_INT, p->tok.data, p->tok.line);
  case T_FLOAT:
    return newNode(N_FLOAT, p->tok.data, p->tok.line);
  case T_CHAR:
    return newNode(N_CHAR, p->tok.data, p->tok.line);
  case T_STRING:
    return newNode(N_STRING, p->tok.data, p->tok.line);
  case T_IDENTIFIER:
    return newNode(N_IDENTIFIER, p->tok.data, p->tok.line);
  case T_TRUE:
    return newNode(N_TRUE, NULL, p->tok.line);
  case T_FALSE:
    return newNode(N_FALSE, NULL, p->tok.line);
  case T_MAKE:
    return parseMakeArray(p);
  case T_CALL:
    return parseFuncCall(p);
  case T_NEW:
    return parseStructNew(p);
  case T_L_PAREN:
    return parseBracketedValue(p);

  default:
    throwParserError(p, "value");
    return ZERO_NODE;
  }
}

Node parseSwitchState(Parser *p) {
  Node out =
      newNode(N_SWITCH_STATE, strNew("Switch Statement", false), p->tok.line);

  CHECK_APPEND_NEXT(T_SWITCH, "switch", N_SWITCH, NULL, "parseSwitchStatement")
  CHECK_APPEND_NEXT(T_L_PAREN, "(", N_L_PAREN, NULL, "parseSwitchStatement")

  if (NodeListAppend(&out.children, parseExpression(p))) {
    panic("Couldn't append to Node list in "
          "parseSwitchStatement");
  }
  APPEND_STRUCTURE(parseExpression, "parseSwitchStatement");
  nextToken(p);

  CHECK_APPEND_NEXT(T_R_PAREN, ")", N_R_PAREN, NULL, "parseSwitchStatement")
  CHECK_APPEND_NEXT(T_L_SQUIRLY, "{", N_L_SQUIRLY, NULL, "parseSwitchStatement")

  while (p->tok.kind == T_CASE) {
    APPEND_STRUCTURE(parseCaseBlock, "parseSwitchStatement");
    nextToken(p);
  }

  if (p->tok.kind == T_DEFAULT) {
    APPEND_STRUCTURE(parseDefaultBlock, "parseSwitchStatement");
    nextToken(p);
  }

  CHECK_AND_APPEND(T_R_SQUIRLY, "}", N_R_SQUIRLY, NULL, "parseSwitchStatement")

  return out;
}

Node parseCaseBlock(Parser *p) {
  Node out = newNode(N_CASE_BLOCK, strNew("Case Block", false), p->tok.line);

  CHECK_APPEND_NEXT(T_CASE, "case", N_CASE, NULL, "parseCaseBlock")

  APPEND_STRUCTURE(parseExpression, "parseCaseBlock");
  nextToken(p);

  CHECK_APPEND_NEXT(T_COLON, ":", N_COLON, NULL, "parseCaseBlock")

  while (p->tok.kind != T_CASE && p->tok.kind != T_DEFAULT &&
         p->tok.kind != T_R_SQUIRLY) {
    switch (p->tok.kind) {
      // Statements
    case T_CALL:
      APPEND_STRUCTURE(parseLoneCall, "parseCaseBlock");
      break;
    case T_LET:
      APPEND_STRUCTURE(parseVarDeclaration, "parseCaseBlock");
      break;
    case T_INC:
    case T_DEC:
    case T_IDENTIFIER:
      APPEND_STRUCTURE(parseVarDeclaration, "parseCaseBlock");
      break;
    case T_IF:
      APPEND_STRUCTURE(parseIfBlock, "parseCaseBlock");
      break;
    case T_FOR:
      APPEND_STRUCTURE(parseForLoop, "parseCaseBlock");
      break;
    case T_RETURN:
      APPEND_STRUCTURE(parseRetState, "parseCaseBlock");
      break;
    case T_BREAK:
      APPEND_STRUCTURE(parseBreakState, "parseCaseBlock");
      break;
    case T_CONTINUE:
      APPEND_STRUCTURE(parseContinueState, "parseCaseBlock");
      break;
    case T_SWITCH:
      APPEND_STRUCTURE(parseSwitchState, "parseCaseBlock");
      break;
    default:
      throwParserError(p, "Valid start to line");
    }

    nextToken(p);
  }

  return out;
}

Node parseDefaultBlock(Parser *p) {
  Node out =
      newNode(N_DEFAULT_BLOCK, strNew("Default Block", false), p->tok.line);

  CHECK_APPEND_NEXT(T_DEFAULT, "default", N_DEFAULT, NULL, "parseDefaultBlock")
  CHECK_APPEND_NEXT(T_COLON, ":", N_COLON, NULL, "parseDefaultBlock")

  while (p->tok.kind != T_R_SQUIRLY) {
    switch (p->tok.kind) {
      // Statements
    case T_CALL:
      break;
    case T_LET:
      break;
    case T_IDENTIFIER:
      break;
    case T_IF:
      break;
    case T_FOR:
      break;
    case T_RETURN:
      break;
    case T_BREAK:
      break;
    case T_CONTINUE:
      break;
    case T_SWITCH:
      break;
    default:
      throwParserError(p, "Valid start to line");
    }

    nextToken(p);
  }

  return out;
}

Node parseBlock(Parser *p) {
  Node out = newNode(N_BLOCK, strNew("Block", false), p->tok.line);

  CHECK_APPEND_NEXT(T_L_SQUIRLY, "{", N_L_SQUIRLY, NULL, "parseBlock")

  while (p->tok.kind != T_R_SQUIRLY) {
    switch (p->tok.kind) {
      // Statements
    case T_CALL:
      APPEND_STRUCTURE(parseLoneCall, "parseBlock");
      break;
    case T_LET:
      APPEND_STRUCTURE(parseVarDeclaration, "parseBlock");
      break;
    case T_INC:
    case T_DEC:
    case T_IDENTIFIER:
      APPEND_STRUCTURE(parseVarDeclaration, "parseBlock");
      break;
    case T_IF:
      APPEND_STRUCTURE(parseIfBlock, "parseBlock");
      break;
    case T_FOR:
      APPEND_STRUCTURE(parseForLoop, "parseBlock");
      break;
    case T_RETURN:
      APPEND_STRUCTURE(parseRetState, "parseBlock");
      break;
    case T_BREAK:
      APPEND_STRUCTURE(parseBreakState, "parseBlock");
      break;
    case T_CONTINUE:
      APPEND_STRUCTURE(parseContinueState, "parseBlock");
      break;
    case T_SWITCH:
      APPEND_STRUCTURE(parseSwitchState, "parseBlock");
      break;
    default:
      throwParserError(p, "Valid start to line");
    }

    nextToken(p);
  }

  return out;

  CHECK_AND_APPEND(T_R_SQUIRLY, "}", N_R_SQUIRLY, NULL, "parseBlock")
}

// The main program
void parse(Parser *p) {
  Node out = newNode(N_PROGRAM, NULL, 0);
  Node n;

  nextToken(p);

  while (p->tok.kind != T_ILLEGAL) {
    switch (p->tok.kind) {
    case T_ENUM:
      APPEND_STRUCTURE(parseEnum, "parse");
      break;
    case T_FUN:
      APPEND_STRUCTURE(parseFunc, "parse");
      break;
    case T_STRUCT:
      APPEND_STRUCTURE(parseStruct, "parse");
      break;

      // All statements will be in functions (main)

    default:
      throwParserError(p, "Valid start to line");
    }

    nextToken(p);
  }

  p->out = out;
}
