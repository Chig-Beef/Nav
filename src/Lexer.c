#include "Panic.h"
#include "Token.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

NEW_LIST_TYPE(Token, Token)

typedef struct Lexer {
  char *sourceName;
  char *source;
  int len;
  int index;
  int line;
} Lexer;

char peekChar(Lexer *l) {
  int nextIndex = l->index + 1;

  if (nextIndex >= l->len) {
    return 0;
  }

  return l->source[nextIndex];
}

void throwError(Lexer *l, char caller[], int line, char expected[], char *got) {
  printf("Error in the Lexer!\nWhen the Lexer was trying to decipher: "
         "%s\nError found in file: %s\nOn line: %i\nExpected: %s\nGot: %s\n",
         caller, l->sourceName, line, expected, got);
  exit(1);
}

// Returns the tokens of the source in the lexer, the resulting list must be
// destroyed
TokenList lex(Lexer *l) {
  TokenList tokens;
  if (TokenListInit(&tokens, 8)) {
    panic("Couldn't initialise token list.");
  }
  Token token = {NULL, T_ILLEGAL, 0};
  char p;

  for (l->index = 0; l->index < l->len; ++l->index) {
    switch (l->source[l->index]) {
      // Skip these
    case ' ':
      continue;
    case '\t':
      continue;
    case '\n':
      ++l->line;
      continue;
    case '\r':
      continue;

    // Single character
    case '.':
      token = (Token){NULL, T_ACCESS, l->line};
    case ':':
      token = (Token){NULL, T_COLON, l->line};
    case ';':
      token = (Token){NULL, T_SEMICOLON, l->line};
    case ',':
      token = (Token){NULL, T_SEP, l->line};
    case '%':
      token = (Token){NULL, T_MOD, l->line};
    case '*':
      token = (Token){NULL, T_MUL, l->line};
    case '~':
      token = (Token){NULL, T_XOR, l->line};
    case '^':
      token = (Token){NULL, T_XOR, l->line};
    case '`':
      token = (Token){NULL, T_XOR, l->line};
    case '[':
      token = (Token){NULL, T_XOR, l->line};
    case ']':
      token = (Token){NULL, T_XOR, l->line};
    case '{':
      token = (Token){NULL, T_XOR, l->line};
    case '}':
      token = (Token){NULL, T_XOR, l->line};
    case '(':
      token = (Token){NULL, T_XOR, l->line};
    case ')':
      token = (Token){NULL, T_XOR, l->line};

      // Could be double
    case '=':
      if (peekChar(l) == '=') {
        ++l->index;
        token = (Token){"==", T_EQ, l->line};
      } else {
        token = (Token){NULL, T_ASSIGN, l->line};
      }
    case '+':
      if (peekChar(l) == '+') {
        ++l->index;
        token = (Token){"++", T_INC, l->line};
      } else {
        token = (Token){NULL, T_ADD, l->line};
      }
    case '&':
      if (peekChar(l) == '&') {
        ++l->index;
        token = (Token){"&&", T_ANDAND, l->line};
      } else {
        token = (Token){NULL, T_AND, l->line};
      }
    case '!':
      if (peekChar(l) == '=') {
        ++l->index;
        token = (Token){"!=", T_NEQ, l->line};
      } else {
        token = (Token){NULL, T_NOT, l->line};
      }
    case '|':
      if (peekChar(l) == '|') {
        ++l->index;
        token = (Token){"||", T_OROR, l->line};
      } else {
        token = (Token){NULL, T_OR, l->line};
      }
    case '-':
      if (peekChar(l) == '-') {
        ++l->index;
        token = (Token){"--", T_DEC, l->line};
      } else {
        token = (Token){NULL, T_SUB, l->line};
      }

      // Slightly larger cases
    case '>':
      p = peekChar(l);
      if (p == '>') {
        ++l->index;
        token = (Token){">>", T_R_SHIFT, l->line};
      } else if (p == '=') {
        ++l->index;
        token = (Token){">=", T_GTEQ, l->line};
      } else {
        token = (Token){NULL, T_GT, l->line};
      }
    case '<':
      p = peekChar(l);
      if (p == '<') {
        ++l->index;
        token = (Token){"<<", T_L_SHIFT, l->line};
      } else if (p == '=') {
        ++l->index;
        token = (Token){"<=", T_LTEQ, l->line};
      } else {
        token = (Token){NULL, T_LT, l->line};
      }

      // Comment, divide
    case '/':
      if (p == '/') {
        while (l->source[l->index] != '\n' && l->index < l->len && l->source) {
          ++l->source;
        }
        continue;
      } else if (p == '*') {
        ++l->source;
        while (l->source[l->index] != '/' && l->source[l->index - 1] != '*' &&
               l->index < l->len && l->source) {
          ++l->source;
        }
        continue;
      } else {
        token = (Token){NULL, T_DIV, l->line};
      }

      // Character
    case '\'':
      // Get length of the char
      int startIndex = l->index;
      ++l->index;
      while (l->source[l->index] != '\'') {
        ++l->index;
      }
      int charLen = l->index - startIndex + 1;
        
        // Copy over the text
      char *text = malloc((charLen + 1) * sizeof(char));
      for (int i = 0; i < charLen; ++i) {
        text[i] = l->source[startIndex + i];
      }

        // Null terminator
      text[charLen] = 0;

    // String
    case '"':
      // Get length of the string
      int startIndex = l->index;
      ++l->index;
      while (l->source[l->index] != '\"') {
        ++l->index;
      }
      int strLen = l->index - startIndex + 1;
        
        // Copy over the text
      char *text = malloc((strLen + 1) * sizeof(char));
      for (int i = 0; i < strLen; ++i) {
        text[i] = l->source[startIndex + i];
      }

        // Null terminator
      text[strLen] = 0;
    }

    // Add that token to the end of the list
    if (TokenListAppend(&tokens, token)) {
      panic("Couldn't append to token list.");
    }

    // Clear the token
    token = (Token){NULL, T_ILLEGAL, 0};
  }

  return tokens;
}
