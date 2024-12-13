#include "Token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Token {
  char *data;
  TokenCode kind;
  int line; // The line this token came from
} Token;

void tokenDestroy(Token *t) { free(t->data); }

char *tokenCodeString(TokenCode tc) {
  switch (tc) {
  case T_ILLEGAL:
    return "T_ILLEGAL";

    // Keywords
  case T_BREAK:
    return "T_BREAK";
  case T_CALL:
    return "T_CALL";
  case T_CASE:
    return "T_CASE";
  case T_CONST:
    return "T_CONST";
  case T_CONTINUE:
    return "T_CONTINUE";
  case T_DEFAULT:
    return "T_DEFAULT";
  case T_ELIF:
    return "T_ELIF";
  case T_ELSE:
    return "T_ELSE";
  case T_ENUM:
    return "T_ENUM";
  case T_FOR:
    return "T_FOR";
  case T_FUN:
    return "T_FUN";
  case T_IF:
    return "T_IF";
  case T_MAKE:
    return "T_MAKE";
  case T_NEW:
    return "T_NEW";
  case T_RETURN:
    return "T_RETURN";
  case T_STRUCT:
    return "T_STRUCT";
  case T_SWITCH:
    return "T_SWITCH";

    // Various symbols
  case T_ACCESS:
    return "T_ACCESS";
  case T_ASSIGN:
    return "T_ASSIGN";
  case T_COLON:
    return "T_COLON";
  case T_SEMICOLON:
    return "T_SEMICOLON";
  case T_SEP:
    return "T_SEP";

    // Operators
  case T_ADD:
    return "T_ADD";
  case T_AND:
    return "T_AND";
  case T_ANDAND:
    return "T_ANDAND";
  case T_DIV:
    return "T_DIV";
  case T_EQ:
    return "T_EQ";
  case T_GT:
    return "T_GT";
  case T_GTEQ:
    return "T_GTEQ";
  case T_LT:
    return "T_LT";
  case T_LTEQ:
    return "T_LTEQ";
  case T_MOD:
    return "T_MOD";
  case T_MUL:
    return "T_MUL";
  case T_NEQ:
    return "T_NEQ";
  case T_OR:
    return "T_OR";
  case T_OROR:
    return "T_OROR";
  case T_SUB:
    return "T_SUB";
  case T_XOR:
    return "T_XOR";
  case T_L_SHIFT:
    return "T_L_SHIFT";
  case T_R_SHIFT:
    return "T_R_SHIFT";

    // Unary operators
  case T_DEREF:
    return "T_DEREF";
  case T_DEC:
    return "T_DEC";
  case T_INC:
    return "T_INC";
  case T_NOT:
    return "T_NOT";
  case T_REF:
    return "T_REF";

    // Paired symbols
  case T_L_BLOCK:
    return "T_L_BLOCK";
  case T_R_BLOCK:
    return "T_R_BLOCK";
  case T_L_SQUIRLY:
    return "T_L_SQUIRLY";
  case T_R_SQUIRLY:
    return "T_R_SQUIRLY";
  case T_L_PAREN:
    return "T_L_PAREN";
  case T_R_PAREN:
    return "T_R_PAREN";

    // Values
  case T_BOOL:
    return "T_BOOL";
  case T_CHAR:
    return "T_CHAR";
  case T_FLOAT:
    return "T_FLOAT";
  case T_IDENTIFIER:
    return "T_IDENTIFIER";
  case T_INT:
    return "T_INT";
  case T_NIL:
    return "T_NIL";
  case T_STRING:
    return "T_STRING";

  default:
    return "UNKNOWN";
  }
}

// Returns a string describing the token. The resulting string must be freed.
char *tokenString(Token *t) {
  // Get text of token code
  char *kind = tokenCodeString(t->kind);

  // The final string
  char *out = malloc((strlen(kind) + strlen(t->data) + 4) * sizeof(char));

  // Format
  sprintf(out, "(%s %s)", t->data, kind);

  return out;
}
