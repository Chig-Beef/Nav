#include "Node.h"
#include "Panic.h"
#include "Token.h"
#include "list.h"
#include <stdio.h>

NEW_LIST_TYPE(Token, Token)

#define CHECK_TOK(tokenCode, expected)                                         \
  if (p->tok.kind != (tokenCode)) {                                            \
    throwError(p, (expected));                                                 \
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

void throwError(Parser *p, char expected[]) {
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
      throwError(p, "Valid start to line");
    }

    nextToken(p);
  }

  return program;
}

Node parseStruct(Parser *p) {
  Node out = newNode(N_STRUCT_DEF, "Struct Def", p->tok.line);

  CHECK_TOK(T_STRUCT, "struct")
  if (NodeListAppend(&out.children, newNode(N_STRUCT, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  if (NodeListAppend(&out.children,
                     newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  CHECK_TOK(T_L_SQUIRLY, "{")
  if (NodeListAppend(&out.children, newNode(N_L_SQUIRLY, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  // TODO: ComplexType

  CHECK_TOK(T_IDENTIFIER, "identifier")
  if (NodeListAppend(&out.children,
                     newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  while (p->tok.kind == T_SEP) {
    if (NodeListAppend(&out.children, newNode(N_SEP, NULL, p->tok.line))) {
      panic("Couldn't append to Node list in parseStruct");
    }
    nextToken(p);

    // TODO: ComplexType

    CHECK_TOK(T_IDENTIFIER, "identifier")
    if (NodeListAppend(&out.children,
                       newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
      panic("Couldn't append to Node list in parseStruct");
    }
    nextToken(p);
  }

  if (p->tok.kind == T_SEP) {
    if (NodeListAppend(&out.children, newNode(N_SEP, NULL, p->tok.line))) {
      panic("Couldn't append to Node list in parseStruct");
    }
    nextToken(p);
  }

  CHECK_TOK(T_R_SQUIRLY, "}")
  if (NodeListAppend(&out.children, newNode(N_R_SQUIRLY, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  return out;
}

Node parseEnum(Parser *p) {
  Node out = newNode(N_ENUM_DEF, "Enum Def", p->tok.line);

  CHECK_TOK(T_ENUM, "enum")
  if (NodeListAppend(&out.children, newNode(N_ENUM, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  if (NodeListAppend(&out.children,
                     newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  CHECK_TOK(T_L_SQUIRLY, "{")
  if (NodeListAppend(&out.children, newNode(N_L_SQUIRLY, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  if (NodeListAppend(&out.children,
                     newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  while (p->tok.kind == T_SEP) {
    if (NodeListAppend(&out.children, newNode(N_SEP, NULL, p->tok.line))) {
      panic("Couldn't append to Node list in parseEnum");
    }
    nextToken(p);

    // TODO: ComplexType

    CHECK_TOK(T_IDENTIFIER, "identifier")
    if (NodeListAppend(&out.children,
                       newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
      panic("Couldn't append to Node list in parseEnum");
    }
    nextToken(p);
  }

  if (p->tok.kind == T_SEP) {
    if (NodeListAppend(&out.children, newNode(N_SEP, NULL, p->tok.line))) {
      panic("Couldn't append to Node list in parseEnum");
    }
    nextToken(p);
  }

  CHECK_TOK(T_R_SQUIRLY, "}")
  if (NodeListAppend(&out.children, newNode(N_R_SQUIRLY, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  return out;
}

Node parseFunc(Parser *p) {
  Node out = newNode(N_FUNC_DEF, "Func Def", p->tok.line);

  CHECK_TOK(T_FUN, "fun")
  if (NodeListAppend(&out.children, newNode(N_FUN, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
  nextToken(p);

  CHECK_TOK(T_IDENTIFIER, "identifier")
  if (NodeListAppend(&out.children,
                     newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
  nextToken(p);

  CHECK_TOK(T_L_PAREN, "(")
  if (NodeListAppend(&out.children, newNode(N_L_PAREN, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
  nextToken(p);

  // First param
  if (p->tok.kind != T_SEP && p->tok.kind != T_R_PAREN) {
    // TODO: Complex type

    CHECK_TOK(T_IDENTIFIER, "identifier")
    if (NodeListAppend(&out.children,
                       newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
      panic("Couldn't append to Node list in parseFunc");
    }
    nextToken(p);
  }

  while (p->tok.kind == T_SEP) {
    if (NodeListAppend(&out.children, newNode(N_SEP, NULL, p->tok.line))) {
      panic("Couldn't append to Node list in parseFunc");
    }
    nextToken(p);

    // TODO: Complex type

    CHECK_TOK(T_IDENTIFIER, "identifier")
    if (NodeListAppend(&out.children,
                       newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
      panic("Couldn't append to Node list in parseFunc");
    }
    nextToken(p);
  }

  CHECK_TOK(T_R_PAREN, ")")
  if (NodeListAppend(&out.children, newNode(N_R_PAREN, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
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
    throwError(p, "operator");

    // This return never occurs
    return ZERO_NODE;
  }
}

Node parseIndex(Parser *p) {
  Node out = newNode(N_INDEX, "Index", p->tok.line);

  CHECK_TOK(T_L_BLOCK, "[")
  if (NodeListAppend(&out.children, newNode(N_L_BLOCK, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
  nextToken(p);

  // TODO: Expression

  CHECK_TOK(T_R_BLOCK, "]")
  if (NodeListAppend(&out.children, newNode(N_R_BLOCK, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }

  return out;
}
