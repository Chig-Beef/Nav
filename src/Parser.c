#include "Node.h"
#include "Panic.h"
#include "Token.h"
#include <stdio.h>

#define CHECK_TOK(tokenCode, expected)                                         \
  if (p->tok.kind != (tokenCode)) {                                            \
    throwParserError(p, (expected));                                           \
  }

#define APPEND_NODE(tokenCode, tokenData, funcName)                            \
  if (NodeListAppend(&out.children,                                            \
                     newNode((tokenCode), (tokenData), p->tok.line))) {        \
    panic("Couldn't append to Node list in " funcName);                        \
  }

typedef struct Parser {
  char *sourceName;
  TokenList source;
  Token tok;
  int index;
} Parser;

void nextToken(Parser *p) {
  Token t = {NULL, T_ILLEGAL, 0};

  if (p->index < p->source.len) {
    t = p->source.p[p->index];
  }

  ++p->index;
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
         p->sourceName, p->tok.line, expected, tokenString(&p->tok));
  exit(1);
}

// The main program
Node parse(Parser *p) {
  const char FUNC_NAME[] = "program";

  Node program = newNode(N_PROGRAM, NULL, 0);
  Node n;

  nextToken(p);

  while (p->tok.kind != T_ILLEGAL) {
    switch (p->tok.kind) {
    case T_ENUM:
      break;
    case T_FUN:
      break;
    case T_STRUCT:
      break;

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

  return program;
}

Node parseStruct(Parser *p) {
  Node out = newNode(N_STRUCT_DEF, "Struct Def", p->tok.line);

  CHECK_TOK(T_STRUCT, "struct")
  APPEND_NODE(N_STRUCT, NULL, "parseStruct")
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseStruct")
  nextToken(p);

  CHECK_TOK(T_L_SQUIRLY, "{")
  APPEND_NODE(N_L_SQUIRLY, NULL, "parseStruct")
  nextToken(p);

  // TODO: ComplexType

  CHECK_TOK(T_IDENTIFIER, "identifier")
  APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseStruct")
  nextToken(p);

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseStruct")
    nextToken(p);

    // TODO: ComplexType

    CHECK_TOK(T_IDENTIFIER, "identifier")
    APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseStruct")
    nextToken(p);
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseStruct")
    nextToken(p);
  }

  CHECK_TOK(T_R_SQUIRLY, "}")
  APPEND_NODE(N_R_SQUIRLY, NULL, "parseStruct")
  nextToken(p);

  return out;
}

Node parseEnum(Parser *p) {
  Node out = newNode(N_ENUM_DEF, "Enum Def", p->tok.line);

  CHECK_TOK(T_ENUM, "enum")
  APPEND_NODE(N_ENUM, NULL, "parseEnum")
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseEnum")
  nextToken(p);

  CHECK_TOK(T_L_SQUIRLY, "{")
  APPEND_NODE(N_L_SQUIRLY, NULL, "parseEnum")
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseEnum")
  nextToken(p);

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseEnum")
    nextToken(p);

    // TODO: ComplexType

    CHECK_TOK(T_IDENTIFIER, "identifier")
    APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseEnum")
    nextToken(p);
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseEnum")
    nextToken(p);
  }

  CHECK_TOK(T_R_SQUIRLY, "}")
  APPEND_NODE(N_R_SQUIRLY, NULL, "parseEnum")
  nextToken(p);

  return out;
}

Node parseFunc(Parser *p) {
  Node out = newNode(N_FUNC_DEF, "Func Def", p->tok.line);

  CHECK_TOK(T_FUN, "fun")
  APPEND_NODE(N_FUN, NULL, "parseFunc")
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseFunc")
  nextToken(p);

  CHECK_TOK(T_L_PAREN, "(")
  APPEND_NODE(N_L_PAREN, NULL, "parseFunc")
  nextToken(p);

  // First param
  if (p->tok.kind != T_SEP && p->tok.kind != T_R_PAREN) {
    // TODO: Complex type

    CHECK_TOK(T_IDENTIFIER, "identifier")
    APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseFunc")
    nextToken(p);
  }

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseFunc")
    nextToken(p);

    // TODO: Complex type

    CHECK_TOK(T_IDENTIFIER, "identifier")
    APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseFunc")
    nextToken(p);
  }

  CHECK_TOK(T_R_PAREN, ")")
  APPEND_NODE(N_R_PAREN, NULL, "parseFunc")
  nextToken(p);

  // TODO: Maybe complex type

  // TODO: Block

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
    throwParserError(p, "operator");

    // This return never occurs
    return ZERO_NODE;
  }
}

Node parseIndex(Parser *p) {
  Node out = newNode(N_INDEX, "Index", p->tok.line);

  CHECK_TOK(T_L_BLOCK, "[")
  APPEND_NODE(N_L_BLOCK, NULL, "parseIndex")
  nextToken(p);

  // TODO: Expression

  CHECK_TOK(T_R_BLOCK, "]")
  APPEND_NODE(N_R_BLOCK, NULL, "parseIndex")

  return out;
}

Node parseIfBlock(Parser *p) {
  Node out = newNode(N_IF_BLOCK, "If Block", p->tok.line);

  CHECK_TOK(T_IF, "if")
  APPEND_NODE(N_IF, NULL, "parseIfBlock")
  nextToken(p);

  CHECK_TOK(T_L_PAREN, "(")
  APPEND_NODE(N_L_PAREN, NULL, "parseIfBlock")
  nextToken(p);

  // TODO: Expression

  CHECK_TOK(T_R_PAREN, ")")
  APPEND_NODE(N_R_PAREN, NULL, "parseIfBlock")
  nextToken(p);

  // TODO: Block

  if (peekToken(p).kind == T_ELIF) {
    nextToken(p);

    APPEND_NODE(N_ELIF, NULL, "parseIfBlock")
    nextToken(p);

    CHECK_TOK(T_L_PAREN, "(")
    APPEND_NODE(N_L_PAREN, NULL, "parseIfBlock")
    nextToken(p);

    // TODO: Expression

    CHECK_TOK(T_R_PAREN, ")")
    APPEND_NODE(N_R_PAREN, NULL, "parseIfBlock")
    nextToken(p);

    // TODO: Block
  }

  if (peekToken(p).kind == T_ELSE) {
    APPEND_NODE(N_ELSE, NULL, "parseIfBlock")

    // TODO: Block
  }

  return out;
}

Node parseForLoop(Parser *p) {
  Node out = newNode(N_FOR_LOOP, "For Loop", p->tok.line);

  CHECK_TOK(T_FOR, "for")
  APPEND_NODE(N_FOR, NULL, "parseForLoop")
  nextToken(p);

  // TODO: possible Assignment or newassignment

  CHECK_TOK(T_SEMICOLON, ";")
  APPEND_NODE(N_SEMICOLON, NULL, "parseForLoop")
  nextToken(p);

  // TODO: possible Expression

  CHECK_TOK(T_SEMICOLON, ";")
  APPEND_NODE(N_SEMICOLON, NULL, "parseForLoop")
  nextToken(p);

  // TODO: Possible assignment

  // TODO: Block

  return out;
}

Node parseRetState(Parser *p) {
  Node out = newNode(N_RET_STATE, "Ret State", p->tok.line);

  CHECK_TOK(T_RETURN, "return")
  APPEND_NODE(N_RETURN, NULL, "parseRetState")
  nextToken(p);

  if (p->tok.kind != ';') {
    // TODO: Expression
  }

  CHECK_TOK(T_SEMICOLON, ";")
  APPEND_NODE(N_SEMICOLON, NULL, "parseRetState")

  return out;
}

Node parseBreakState(Parser *p) {
  Node out = newNode(N_BREAK_STATE, "Break State", p->tok.line);

  CHECK_TOK(T_BREAK, "break")
  APPEND_NODE(N_BREAK, NULL, "parseBreakState")
  nextToken(p);

  CHECK_TOK(T_SEMICOLON, ";")
  APPEND_NODE(N_SEMICOLON, NULL, "parseBreakState")

  return out;
}

Node parseContinueState(Parser *p) {
  Node out = newNode(N_CONTINUE_STATE, "Continue State", p->tok.line);

  CHECK_TOK(T_CONTINUE, "continue")
  APPEND_NODE(N_CONTINUE, NULL, "parseContinueState")
  nextToken(p);

  CHECK_TOK(T_SEMICOLON, ";")
  APPEND_NODE(N_SEMICOLON, NULL, "parseContinueState")

  return out;
}

Node parseBracketedValue(Parser *p) {
  Node out = newNode(N_BRACKETED_VALUE, "Bracketed Value", p->tok.line);

  CHECK_TOK(T_L_PAREN, "(")
  APPEND_NODE(N_L_PAREN, NULL, "parseBracketedValue")
  nextToken(p);

  // TODO: Expression

  CHECK_TOK(T_R_PAREN, ")")
  APPEND_NODE(N_R_PAREN, NULL, "parseBracketedValue")

  return out;
}

Node parseStructNew(Parser *p) {
  Node out = newNode(N_STRUCT_NEW, "Struct New", p->tok.line);

  CHECK_TOK(T_STRUCT, "struct")
  APPEND_NODE(N_STRUCT, NULL, "parseStructNew")
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  APPEND_NODE(N_IDENTIFIER, p->tok.data, "parseStructNew")
  nextToken(p);

  CHECK_TOK(T_L_PAREN, "(")
  APPEND_NODE(N_L_PAREN, NULL, "parseStructNew")
  nextToken(p);

  // TODO: expression

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseStructNew")
    nextToken(p);

    // TODO: expression
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseStructNew")
    nextToken(p);
  }

  CHECK_TOK(T_R_PAREN, ")")
  APPEND_NODE(N_R_PAREN, NULL, "parseStructNew")

  return out;
}

Node parseFuncCall(Parser *p) {
  Node out = newNode(N_FUNC_CALL, "Func Call", p->tok.line);

  CHECK_TOK(T_FUN, "fun")
  APPEND_NODE(N_FUN, NULL, "parseFuncCall")
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  APPEND_NODE(N_IDENTIFIER, NULL, "parseFuncCall")
  nextToken(p);

  CHECK_TOK(T_L_PAREN, "(")
  APPEND_NODE(N_L_PAREN, NULL, "parseFuncCall")
  nextToken(p);

  // TODO: expression

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseFuncCall")
    nextToken(p);

    // TODO: expression
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseFuncCall")
    nextToken(p);
  }

  CHECK_TOK(T_R_PAREN, ")")
  APPEND_NODE(N_R_PAREN, NULL, "parseFuncCall")

  return out;
}

Node parseMakeArray(Parser *p) {
  Node out = newNode(N_MAKE_ARRAY, "Make Array", p->tok.line);

  CHECK_TOK(T_MAKE, "make")
  APPEND_NODE(N_MAKE, NULL, "parseMakeArray")
  nextToken(p);

  CHECK_TOK(T_L_BLOCK, "[")
  APPEND_NODE(N_L_BLOCK, NULL, "parseMakeArray")
  nextToken(p);

  // TODO: expression

  while (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseMakeArray")
    nextToken(p);

    // TODO: expression
  }

  if (p->tok.kind == T_SEP) {
    APPEND_NODE(N_SEP, NULL, "parseMakeArray")
    nextToken(p);
  }

  CHECK_TOK(T_R_BLOCK, "]")
  APPEND_NODE(N_R_BLOCK, NULL, "parseMakeArray")
  nextToken(p);

  return out;
}

Node parseLoneCall(Parser *p) {
  Node out = newNode(N_LONE_CALL, "Lone Call", p->tok.line);

  if (NodeListAppend(&out.children, parseFuncCall(p))) {
    panic("Couldn't append to Node list in parseLoneCall");
  }
  nextToken(p);

  CHECK_TOK(T_SEMICOLON, ";")
  APPEND_NODE(N_SEMICOLON, NULL, "parseLoneCall")

  return out;
}
