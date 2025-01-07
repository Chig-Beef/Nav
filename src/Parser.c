#include "Node.h"
#include "Panic.h"
#include "Token.h"
#include "list.h"
#include <stdio.h>

NEW_LIST_TYPE(Token, Token)

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

  if (p->tok.kind != T_STRUCT) {
    throwError(p, "struct");
  }
  if (NodeListAppend(&out.children, newNode(N_STRUCT, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  if (p->tok.kind != T_IDENTIFIER) {
    throwError(p, "identifier");
  }
  if (NodeListAppend(&out.children,
                     newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  if (p->tok.kind != T_L_SQUIRLY) {
    throwError(p, "{");
  }
  if (NodeListAppend(&out.children, newNode(N_L_SQUIRLY, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  // TODO: ComplexType

  if (p->tok.kind != T_IDENTIFIER) {
    throwError(p, "identifier");
  }
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

    if (p->tok.kind != T_IDENTIFIER) {
      throwError(p, "identifier");
    }
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

  if (p->tok.kind != T_R_SQUIRLY) {
    throwError(p, "}");
  }
  if (NodeListAppend(&out.children, newNode(N_R_SQUIRLY, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseStruct");
  }
  nextToken(p);

  return out;
}

Node parseEnum(Parser *p) {
  Node out = newNode(N_ENUM_DEF, "Enum Def", p->tok.line);

  if (p->tok.kind != T_ENUM) {
    throwError(p, "enum");
  }
  if (NodeListAppend(&out.children, newNode(N_ENUM, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  if (p->tok.kind != T_IDENTIFIER) {
    throwError(p, "identifier");
  }
  if (NodeListAppend(&out.children,
                     newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  if (p->tok.kind != T_L_SQUIRLY) {
    throwError(p, "{");
  }
  if (NodeListAppend(&out.children, newNode(N_L_SQUIRLY, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  if (p->tok.kind != T_IDENTIFIER) {
    throwError(p, "identifier");
  }
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

    if (p->tok.kind != T_IDENTIFIER) {
      throwError(p, "identifier");
    }
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

  if (p->tok.kind != T_R_SQUIRLY) {
    throwError(p, "}");
  }
  if (NodeListAppend(&out.children, newNode(N_R_SQUIRLY, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseEnum");
  }
  nextToken(p);

  return out;
}

Node parseFunc(Parser *p) {
  Node out = newNode(N_FUNC_DEF, "Func Def", p->tok.line);

  if (p->tok.kind != T_FUN) {
    throwError(p, "fun");
  }
  if (NodeListAppend(&out.children, newNode(N_FUN, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
  nextToken(p);

  if (p->tok.kind != T_IDENTIFIER) {
    throwError(p, "identifier");
  }
  if (NodeListAppend(&out.children,
                     newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
  nextToken(p);

  if (p->tok.kind != T_L_PAREN) {
    throwError(p, "(");
  }
  if (NodeListAppend(&out.children, newNode(N_L_PAREN, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
  nextToken(p);

  // First param
  if (p->tok.kind != T_SEP && p->tok.kind != T_R_PAREN) {
    // TODO: Complex type

    if (p->tok.kind != T_IDENTIFIER) {
      throwError(p, "identifier");
    }
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

    if (p->tok.kind != T_IDENTIFIER) {
      throwError(p, "identifier");
    }
    if (NodeListAppend(&out.children,
                       newNode(N_IDENTIFIER, p->tok.data, p->tok.line))) {
      panic("Couldn't append to Node list in parseFunc");
    }
    nextToken(p);
  }

  if (p->tok.kind != T_R_PAREN) {
    throwError(p, ")");
  }
  if (NodeListAppend(&out.children, newNode(N_R_PAREN, NULL, p->tok.line))) {
    panic("Couldn't append to Node list in parseFunc");
  }
  nextToken(p);

  // TODO: Maybe complex type

  // TODO: Block

  return out;
}
