#pragma once

#define ZERO_NODE (Node){N_ILLEGAL, ZERO_LIST, NULL, 0}

#include "String.h"
#include "list.h"

typedef enum NodeCode {
  N_ILLEGAL,

  // Structures
  N_PROGRAM,
  N_ENUM_DEF,
  N_FUNC_DEF,
  N_STRUCT_DEF,
  N_COMPLEX_TYPE,
  N_BLOCK,
  N_INDEX,
  N_EXPRESSION,
  N_UNARY_VALUE,
  N_OPERATOR,
  N_BRACKETED_VALUE,
  N_LONE_CALL,
  N_MAKE_ARRAY,
  N_FUNC_CALL,
  N_STRUCT_NEW,
  N_VAR_DEC,
  N_NEW_ASSIGNMENT,
  N_CREMENT,
  N_ASSIGNMENT,
  N_IF_BLOCK,
  N_IF_SEC,
  N_ELIF_SEC,
  N_ELSE_SEC,
  N_FOR_LOOP,
  N_RET_STATE,
  N_BREAK_STATE,
  N_CONTINUE_STATE,
  N_SWITCH_STATE,
  N_CASE_BLOCK,
  N_DEFAULT_BLOCK,
  N_ACCESS,

  // Keywords
  N_BREAK,
  N_CALL,
  N_CASE,
  N_CONST,
  N_CONTINUE,
  N_DEFAULT,
  N_ELIF,
  N_ELSE,
  N_ENUM,
  N_FOR,
  N_FUN,
  N_IF,
  N_LET,
  N_MAKE,
  N_NEW,
  N_RETURN,
  N_STRUCT,
  N_SWITCH,
  N_TRUE,
  N_FALSE,

  // Various symbols
  N_ACCESSOR,
  N_P_ACCESSOR,
  N_ASSIGN,
  N_COLON,
  N_SEMICOLON,
  N_SEP,

  // Operators
  N_ADD,
  N_AND,
  N_ANDAND,
  N_DIV,
  N_EQ,
  N_GT,
  N_GTEQ,
  N_LT,
  N_LTEQ,
  N_MOD,
  N_MUL,
  N_NEQ,
  N_OR,
  N_OROR,
  N_SUB,
  N_XOR,
  N_L_SHIFT,
  N_R_SHIFT,

  // Unary operators
  N_DEREF,
  N_DEC,
  N_INC,
  N_NOT,
  N_REF,

  // Paired symbols
  N_L_BLOCK,   // [
  N_R_BLOCK,   // ]
  N_L_SQUIRLY, // {
  N_R_SQUIRLY, // }
  N_L_PAREN,   // (
  N_R_PAREN,   // )

  // Values
  N_CHAR,
  N_FLOAT,
  N_IDENTIFIER,
  N_INT,
  N_STRING,
} NodeCode;

typedef struct Node Node;

NEW_LIST_TYPE_HEADER(Node, Node)

struct Node {
  NodeCode kind;
  NodeList children;
  String *data;
  int line;
  char *sourceName;
};

void nodeDestroy(Node *t);

char *nodeCodeString(NodeCode tc);

// Returns a string describing the token. The resulting string must be freed.
char *nodeString(Node *t);

Node newNode(NodeCode kind, String *data, int line, char *sourceName);
