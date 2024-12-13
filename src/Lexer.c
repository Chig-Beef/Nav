#include "Panic.h"
#include "Token.h"
#include "list.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define IS_DIGIT(VAL) ((VAL) >= '0' && (VAL) <= '9')
#define IS_ALPHA(VAL)                                                          \
  ((VAL) >= 'a' && (VAL) <= 'z') || ((VAL) >= 'A' && (VAL) <= 'Z')
#define VALID_IDENT_CHAR(VAL) (IS_ALPHA(VAL) || IS_DIGIT(VAL) || (VAL) == '_')
#define VALID_NUM_CHAR(VAL) (IS_DIGIT(VAL) || (VAL) == '.')

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

void throwError(Lexer *l, int line, char expected[], char got) {
  printf("Error in the Lexer!\n"
         "Error found in file: %s\nOn line: %i\nExpected: %s\nGot: %c (%i)\n",
         l->sourceName, line, expected, got, got);
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
  char *text;
  int startIndex;

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
      startIndex = l->index;
      ++l->index;
      while (l->source[l->index] != '\'') {
        ++l->index;
      }
      int charLen = l->index - startIndex + 1;

      // Copy over the text
      text = malloc((charLen + 1) * sizeof(char));
      for (int i = 0; i < charLen; ++i) {
        text[i] = l->source[startIndex + i];
      }

      // Null terminator
      text[charLen] = 0;

      // Save the token
      token = (Token){text, T_CHAR, l->line};

    // String
    case '"':
      // Get length of the string
      startIndex = l->index;
      ++l->index;
      while (l->source[l->index] != '\"') {
        ++l->index;
      }
      int strLen = l->index - startIndex + 1;

      // Copy over the text
      text = malloc((strLen + 1) * sizeof(char));
      for (int i = 0; i < strLen; ++i) {
        text[i] = l->source[startIndex + i];
      }

      // Null terminator
      text[strLen] = 0;

      // Save the token
      token = (Token){text, T_STRING, l->line};

    default:
      // Number
      if (IS_DIGIT(l->source[l->index])) {
        // Get length of number
        startIndex = l->index;
        ++l->index;
        bool isFloat = false;
        while (VALID_NUM_CHAR(peekChar(l))) {
          ++l->index;
          if (l->source[l->index] == '.') {
            isFloat = true;
          }
        }
        int numLen = l->index - startIndex + 1;

        // Copy over the text
        text = malloc((numLen + 1) * sizeof(char));
        for (int i = 0; i < numLen; ++i) {
          text[i] = l->source[startIndex + i];
        }

        // Null terminator
        text[strLen] = 0;
        if (text[strLen - 1] == '.') {
          throwError(l, l->line, "Digits after decimal", '.');
        }

        // Save the token
        if (isFloat) {
          token = (Token){text, T_FLOAT, l->line};
        } else {
          token = (Token){text, T_INT, l->line};
        }
      }

      // Keywords & Identifier
      else if (IS_ALPHA(l->source[l->index])) {
      }

      // Bad char
      else {
        throwError(l, l->line, "Any other character", l->source[l->index]);
      }
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
