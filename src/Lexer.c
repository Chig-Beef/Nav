#include "Panic.h"
#include "Token.h"
#include "list.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IS_DIGIT(VAL) ((VAL) >= '0' && (VAL) <= '9')
#define IS_ALPHA(VAL)                                                          \
  ((VAL) >= 'a' && (VAL) <= 'z') || ((VAL) >= 'A' && (VAL) <= 'Z')
#define VALID_IDENT_CHAR(VAL) (IS_ALPHA(VAL) || IS_DIGIT(VAL) || (VAL) == '_')
#define VALID_NUM_CHAR(VAL) (IS_DIGIT(VAL) || (VAL) == '.')
#define STR_BUFF_SIZE 100

NEW_LIST_TYPE(Token, Token)

typedef struct Lexer {
  char *sourceName;
  FILE *source;
  char curChar;
  char peekChar;
  int line;
} Lexer;

void nextChar(Lexer *l) {
  l->curChar = l->peekChar;
  l->peekChar = fgetc(l->source);
  if (l->curChar == '\n') {
    ++l->line;
  }
}

void lexerInit(Lexer *l, char sourceName[], FILE *source) {
  l->sourceName = sourceName;
  l->source = source;
  l->line = 1;
  l->peekChar = 0;
  nextChar(l);
  nextChar(l);
}

void throwError(Lexer *l, char expected[], char got) {
  printf("Error in the Lexer!\n"
         "Error found in file: %s\nOn line: %i\nExpected: %s\nGot: %c (%i)\n",
         l->sourceName, l->line, expected, got, got);
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
  char strBuff[STR_BUFF_SIZE];
  int dynLen; // Length of variable sized token text

  // A continue in this loop means skipping the token append
  while (l->curChar >= 0) {
    switch (l->curChar) {
      // Skip these
    case ' ':
      continue;
    case '\t':
      continue;
    case '\n':
      continue;
    case '\r':
      continue;
    case 0:
      throwError(l, "More source", 0);

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
      token = (Token){NULL, T_DEREF, l->line};
    case '`':
      token = (Token){NULL, T_REF, l->line};
    case '[':
      token = (Token){NULL, T_L_BLOCK, l->line};
    case ']':
      token = (Token){NULL, T_R_BLOCK, l->line};
    case '{':
      token = (Token){NULL, T_L_SQUIRLY, l->line};
    case '}':
      token = (Token){NULL, T_R_SQUIRLY, l->line};
    case '(':
      token = (Token){NULL, T_L_PAREN, l->line};
    case ')':
      token = (Token){NULL, T_R_PAREN, l->line};

      // Could be double
    case '=':
      if (l->peekChar == '=') {
        nextChar(l);
        token = (Token){NULL, T_EQ, l->line};
      } else {
        token = (Token){NULL, T_ASSIGN, l->line};
      }
    case '+':
      if (l->peekChar == '+') {
        nextChar(l);
        token = (Token){NULL, T_INC, l->line};
      } else {
        token = (Token){NULL, T_ADD, l->line};
      }
    case '&':
      if (l->peekChar == '&') {
        nextChar(l);
        token = (Token){NULL, T_ANDAND, l->line};
      } else {
        token = (Token){NULL, T_AND, l->line};
      }
    case '!':
      if (l->peekChar == '=') {
        nextChar(l);
        token = (Token){NULL, T_NEQ, l->line};
      } else {
        token = (Token){NULL, T_NOT, l->line};
      }
    case '|':
      if (l->peekChar == '|') {
        nextChar(l);
        token = (Token){NULL, T_OROR, l->line};
      } else {
        token = (Token){NULL, T_OR, l->line};
      }
    case '-':
      if (l->peekChar == '-') {
        nextChar(l);
        token = (Token){NULL, T_DEC, l->line};
      } else {
        token = (Token){NULL, T_SUB, l->line};
      }

      // Slightly larger cases
    case '>':
      if (l->peekChar == '>') {
        nextChar(l);
        token = (Token){NULL, T_R_SHIFT, l->line};
      } else if (l->peekChar == '=') {
        nextChar(l);
        token = (Token){NULL, T_GTEQ, l->line};
      } else {
        token = (Token){NULL, T_GT, l->line};
      }
    case '<':
      if (l->peekChar == '<') {
        nextChar(l);
        token = (Token){NULL, T_L_SHIFT, l->line};
      } else if (l->peekChar == '=') {
        nextChar(l);
        token = (Token){NULL, T_LTEQ, l->line};
      } else {
        token = (Token){NULL, T_LT, l->line};
      }

      // Comment, divide
    case '/':
      if (l->peekChar == '/') {
        while (l->curChar != '\n' && l->curChar >= 0) {
          nextChar(l);
        }
        nextChar(l); // Now the character for the next loop is the first one of
                     // the next line
        continue;
      } else if (l->peekChar == '*') {
        nextChar(l);
        while ((l->peekChar != '/' || l->curChar != '*') && l->peekChar >= 0) {
          nextChar(l);
        }
        // Now the character for the next loop is the one right after the
        // comment
        nextChar(l);
        nextChar(l);
        continue;
      } else {
        token = (Token){NULL, T_DIV, l->line};
      }

      // Character
    case '\'':
      // Get length of the char
      dynLen = 1;

      // Get the buffer ready
      memset(strBuff, 0, STR_BUFF_SIZE);
      strBuff[0] = l->curChar;
      nextChar(l);

      // Copy char text
      while (l->curChar != '\'' && dynLen < STR_BUFF_SIZE) {
        // Invalid characters in char
        switch (l->curChar) {
        case '\n':
          throwError(l, "No newline in character", '\n');
        case '\r':
          throwError(l, "No return in character", '\r');
        }

        // Copy in next char
        strBuff[dynLen] = l->curChar;
        nextChar(l);
        ++dynLen;

        // End of source?
        if (l->curChar < 0) {
          throwError(l, "More source for character", 0);
        }
      }

      // Empty character
      if (dynLen == 2) {
        throwError(l, "Char in character definition", '\'');
      }

      // Copy over to dynamic allocation, freeing up strBuff for next char,
      // string, etc
      text = malloc((dynLen + 1) * sizeof(char));
      strcpy_s(text, dynLen, strBuff);
      text[dynLen] = 0;

      // Save the token
      token = (Token){text, T_CHAR, l->line};

    // String
    case '"':
      // Get length of the string
      dynLen = 1;

      // Get the buffer ready
      memset(strBuff, 0, STR_BUFF_SIZE);
      strBuff[0] = l->curChar;
      nextChar(l);

      // Copy string text
      while (l->curChar != '\"') {
        // Invalid characters in string
        switch (l->curChar) {
        case '\n':
          throwError(l, "No newline in string", '\n');
        case '\r':
          throwError(l, "No return in string", '\r');
        }

        // Copy in next char
        strBuff[dynLen] = l->curChar;
        nextChar(l);
        ++dynLen;

        // End of source?
        if (l->curChar < 0) {
          throwError(l, "More source for string", 0);
        }
      }

      // Copy over to dynamic allocation, freeing up strBuff for next char,
      // string, etc
      text = malloc((dynLen + 1) * sizeof(char));
      strcpy_s(text, dynLen, strBuff);
      text[dynLen] = 0;

      // Save the token
      token = (Token){text, T_STRING, l->line};

    default:
      // Number
      if (IS_DIGIT(l->curChar)) {
        // Get length of the number
        dynLen = 0;

        // Get the buffer ready
        memset(strBuff, 0, STR_BUFF_SIZE);
        strBuff[0] = l->curChar;

        // Get length of number
        bool isFloat = false;
        while (VALID_NUM_CHAR(l->peekChar)) {
          nextChar(l);
          if (l->curChar == '.') {
            if (isFloat) {
              throwError(l, "Only one decimal in float", '.');
            }
            isFloat = true;
          }
        }

        // Copy over to dynamic allocation, freeing up strBuff for next char,
        // string, etc
        text = malloc((dynLen + 1) * sizeof(char));
        strcpy_s(text, dynLen, strBuff);
        text[dynLen] = 0;

        if (text[dynLen - 1] == '.') {
          throwError(l, "Digits after decimal", '.');
        }

        // Save the token
        if (isFloat) {
          token = (Token){text, T_FLOAT, l->line};
        } else {
          token = (Token){text, T_INT, l->line};
        }
      }

      // Keywords & Identifier
      else if (IS_ALPHA(l->curChar)) {
        // Get length of the identifier
        dynLen = 0;

        // Get the buffer ready
        memset(strBuff, 0, STR_BUFF_SIZE);
        strBuff[0] = l->curChar;

        // Get length of word
        while (VALID_IDENT_CHAR(l->peekChar)) {
          nextChar(l);
        }

        // Save the token
        // NOTE: All of these keywords are known ahead of time, therefore no
        // dynamic allocation is needed
        if (strcmp(text, "break")) {
          token = (Token){NULL, T_BREAK, l->line};
        } else if (strcmp(NULL, "call")) {
          token = (Token){NULL, T_CALL, l->line};
        } else if (strcmp(NULL, "case")) {
          token = (Token){NULL, T_CASE, l->line};
        } else if (strcmp(NULL, "const")) {
          token = (Token){NULL, T_CONST, l->line};
        } else if (strcmp(NULL, "continue")) {
          token = (Token){NULL, T_CONTINUE, l->line};
        } else if (strcmp(NULL, "default")) {
          token = (Token){NULL, T_DEFAULT, l->line};
        } else if (strcmp(NULL, "elif")) {
          token = (Token){NULL, T_ELIF, l->line};
        } else if (strcmp(NULL, "else")) {
          token = (Token){NULL, T_ELSE, l->line};
        } else if (strcmp(NULL, "enum")) {
          token = (Token){NULL, T_ENUM, l->line};
        } else if (strcmp(NULL, "for")) {
          token = (Token){NULL, T_FOR, l->line};
        } else if (strcmp(NULL, "fun")) {
          token = (Token){NULL, T_FUN, l->line};
        } else if (strcmp(NULL, "if")) {
          token = (Token){NULL, T_IF, l->line};
        } else if (strcmp(NULL, "make")) {
          token = (Token){NULL, T_MAKE, l->line};
        } else if (strcmp(NULL, "new")) {
          token = (Token){NULL, T_NEW, l->line};
        } else if (strcmp(NULL, "return")) {
          token = (Token){NULL, T_RETURN, l->line};
        } else if (strcmp(NULL, "struct")) {
          token = (Token){NULL, T_STRUCT, l->line};
        } else if (strcmp(NULL, "switch")) {
          token = (Token){NULL, T_SWITCH, l->line};
        } else if (strcmp(NULL, "nil")) {
          token = (Token){NULL, T_NIL, l->line};
        } else if (strcmp(NULL, "true")) {
          token = (Token){NULL, T_TRUE, l->line};
        } else if (strcmp(NULL, "false")) {
          token = (Token){NULL, T_FALSE, l->line};
        } else {
          // Copy over to dynamic allocation, freeing up strBuff for next char,
          // string, etc
          text = malloc((dynLen + 1) * sizeof(char));
          strcpy_s(text, dynLen, strBuff);
          text[dynLen] = 0;

          token = (Token){text, T_IDENTIFIER, l->line};
        }
      }

      // Bad char
      else {
        throwError(l, "Any other character", l->curChar);
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
