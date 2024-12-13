#pragma once

typedef enum TokenCode {
  T_ILLEGAL,

  // Keywords
  T_BREAK,
  T_CALL,
  T_CASE,
  T_CONST,
  T_CONTINUE,
  T_DEFAULT,
  T_ELIF,
  T_ELSE,
  T_ENUM,
  T_FOR,
  T_FUN,
  T_IF,
  T_MAKE,
  T_NEW,
  T_RETURN,
  T_STRUCT,
  T_SWITCH,

  // Various symbols
  T_ACCESS,
  T_ASSIGN,
  T_COLON,
  T_SEMICOLON,
  T_SEP,

  // Operators
  T_ADD,
  T_AND,
  T_ANDAND,
  T_DIV,
  T_EQ,
  T_GT,
  T_GTEQ,
  T_LT,
  T_LTEQ,
  T_MOD,
  T_MUL,
  T_NEQ,
  T_OR,
  T_OROR,
  T_SUB,
  T_XOR,
  T_L_SHIFT,
  T_R_SHIFT,

  // Unary operators
  T_DEREF,
  T_DEC,
  T_INC,
  T_NOT,
  T_REF,

  // Paired symbols
  T_L_BLOCK,   // [
  T_R_BLOCK,   // ]
  T_L_SQUIRLY, // {
  T_R_SQUIRLY, // }
  T_L_PAREN,   // (
  T_R_PAREN,   // )

  // Values
  T_BOOL,
  T_CHAR,
  T_FLOAT,
  T_IDENTIFIER,
  T_INT,
  T_NIL,
  T_STRING,
} TokenCode;

typedef struct Token {
  char *data;
  TokenCode kind;
  int line; // The line this token came from
} Token;

void tokenDestroy(Token *t);

char *tokenCodeString(TokenCode tc);

// Returns a string describing the token. The resulting string must be freed.
char *tokenString(Token *t);
