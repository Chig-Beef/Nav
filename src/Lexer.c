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
#define VALID_NUM_CHAR(VAL) (IS_DIGIT(VAL) || (VAL) == '.' || (VAL) == '_')
#define STR_BUFF_SIZE 100

NEW_LIST_TYPE(Token, Token)

typedef struct Lexer {
  char *sourceName;
  FILE *source;
  char curChar;
  char peekChar;
  int line;
  TokenList out;
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

  if (TokenListInit(&l->out, 8)) {
    panic("Couldn't initialise token list.");
  }
}

void throwError(Lexer *l, char expected[], char got) {
  printf("Error in the Lexer!\n"
         "Error found in file: %s\nOn line: %i\nExpected: %s\nGot: %c (%i)\n",
         l->sourceName, l->line, expected, got, got);
  exit(1);
}

// Returns the tokens of the source in the lexer, the resulting list must be
// destroyed
void lex(Lexer *l) {
  Token token = {NULL, T_ILLEGAL, 0};
  char p;
  char *text;
  char strBuff[STR_BUFF_SIZE];
  int dynLen; // Length of variable sized token text
  bool escaped;

  // A NO_TOKEN in this loop means skipping the token append
  while (l->curChar >= 0) {
    switch (l->curChar) {
      // Skip these
    case ' ':
      goto NO_TOKEN;
    case '\t':
      goto NO_TOKEN;
    case '\n':
      goto NO_TOKEN;
    case '\r':
      goto NO_TOKEN;
    case 0:
      throwError(l, "More source", 0);

    // Single character
    case '.':
      token = (Token){NULL, T_ACCESS, l->line};
      break;
    case ':':
      token = (Token){NULL, T_COLON, l->line};
      break;
    case ';':
      token = (Token){NULL, T_SEMICOLON, l->line};
      break;
    case ',':
      token = (Token){NULL, T_SEP, l->line};
      break;
    case '%':
      token = (Token){NULL, T_MOD, l->line};
      break;
    case '*':
      token = (Token){NULL, T_MUL, l->line};
      break;
    case '~':
      token = (Token){NULL, T_XOR, l->line};
      break;
    case '^':
      token = (Token){NULL, T_DEREF, l->line};
      break;
    case '`':
      token = (Token){NULL, T_REF, l->line};
      break;
    case '[':
      token = (Token){NULL, T_L_BLOCK, l->line};
      break;
    case ']':
      token = (Token){NULL, T_R_BLOCK, l->line};
      break;
    case '{':
      token = (Token){NULL, T_L_SQUIRLY, l->line};
      break;
    case '}':
      token = (Token){NULL, T_R_SQUIRLY, l->line};
      break;
    case '(':
      token = (Token){NULL, T_L_PAREN, l->line};
      break;
    case ')':
      token = (Token){NULL, T_R_PAREN, l->line};
      break;

      // Could be double
    case '=':
      if (l->peekChar == '=') {
        nextChar(l);
        token = (Token){NULL, T_EQ, l->line};
      } else {
        token = (Token){NULL, T_ASSIGN, l->line};
      }
      break;
    case '+':
      if (l->peekChar == '+') {
        nextChar(l);
        token = (Token){NULL, T_INC, l->line};
      } else {
        token = (Token){NULL, T_ADD, l->line};
      }
      break;
    case '&':
      if (l->peekChar == '&') {
        nextChar(l);
        token = (Token){NULL, T_ANDAND, l->line};
      } else {
        token = (Token){NULL, T_AND, l->line};
      }
      break;
    case '!':
      if (l->peekChar == '=') {
        nextChar(l);
        token = (Token){NULL, T_NEQ, l->line};
      } else {
        token = (Token){NULL, T_NOT, l->line};
      }
      break;
    case '|':
      if (l->peekChar == '|') {
        nextChar(l);
        token = (Token){NULL, T_OROR, l->line};
      } else {
        token = (Token){NULL, T_OR, l->line};
      }
      break;
    case '-':
      if (l->peekChar == '-') {
        nextChar(l);
        token = (Token){NULL, T_DEC, l->line};
      } else {
        token = (Token){NULL, T_SUB, l->line};
      }
      break;

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
      break;
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
      break;

      // Comment, divide
    case '/':
      if (l->peekChar == '/') {
        while (l->curChar != '\n' && l->curChar >= 0) {
          nextChar(l);
        }
        nextChar(l); // Now the character for the next loop is the first one of
                     // the next line
        goto NO_TOKEN;
      } else if (l->peekChar == '*') {
        nextChar(l);
        while ((l->peekChar != '/' || l->curChar != '*') && l->peekChar >= 0) {
          nextChar(l);
        }
        // Now the character for the next loop is the one right after the
        // comment
        nextChar(l);
        nextChar(l);
        goto NO_TOKEN;
      } else {
        token = (Token){NULL, T_DIV, l->line};
        break;
      }

      // Character
    case '\'':
      // Get length of the char
      dynLen = 1;

      // Get the buffer ready
      memset(strBuff, 0, STR_BUFF_SIZE);
      strBuff[0] = '\'';
      nextChar(l);

      switch (l->curChar) {
      case '\'':
        throwError(l, "Char in character definition", '\'');
      case '\n':
        throwError(l, "No newline in character", '\n');
      case '\r':
        throwError(l, "No return in character", '\r');
      case '\\':
        strBuff[dynLen] = l->curChar;
        nextChar(l);
        ++dynLen;

        switch (l->curChar) {
        case '\n':
          throwError(l, "No newline in character", '\n');
        case '\r':
          throwError(l, "No return in character", '\r');
        default:
          strBuff[dynLen] = l->curChar;
          nextChar(l);
          ++dynLen;
        }
        break;

      default:
        strBuff[dynLen] = l->curChar;
        nextChar(l);
        ++dynLen;
      }

      if (l->curChar != '\'') {
        throwError(l, "End of character literal", l->curChar);
      }

      strBuff[dynLen] = '\'';
      ++dynLen;

      // Copy over to dynamic allocation, freeing up strBuff for next char,
      // string, etc
      text = malloc((dynLen + 1) * sizeof(char));
      if (!text) {
        panic("OOM\n");
      }
      if (strcpy_s(text, (unsigned long long)(dynLen + 1), strBuff)) {
        panic("Couldn't copy string\n");
      }

      // Save the token
      token = (Token){text, T_CHAR, l->line};
      break;

    // String
    case '"':
      // Get length of the string
      dynLen = 1;

      // Get the buffer ready
      memset(strBuff, 0, STR_BUFF_SIZE);
      strBuff[0] = l->curChar;
      nextChar(l);

      // Backslash?
      escaped = false;

      // Copy string text
      while (!(l->curChar == '\"' && !escaped) && dynLen < STR_BUFF_SIZE) {
        escaped = false;

        // Invalid characters in string
        switch (l->curChar) {
        case '\n':
          throwError(l, "No newline in string", '\n');
        case '\r':
          throwError(l, "No return in string", '\r');
        case '\\':
          escaped = true;
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
      if (!text) {
        panic("OOM\n");
      }
      if (strcpy_s(text, (unsigned long long)(dynLen + 1), strBuff)) {
        panic("Couldn't copy string\n");
      }
      text[dynLen] = 0;

      // Save the token
      token = (Token){text, T_STRING, l->line};
      break;

    default:
      // Number
      if (IS_DIGIT(l->curChar)) {
        // Get length of the number
        dynLen = 1;

        // Get the buffer ready
        memset(strBuff, 0, STR_BUFF_SIZE);
        strBuff[0] = l->curChar;

        // Get length of number
        bool isFloat = false;
        while (VALID_NUM_CHAR(l->peekChar) && dynLen < STR_BUFF_SIZE) {
          // Don't add underscores to final string
          if (l->curChar == '_') {
            nextChar(l);
            continue;
          }
          // Copy in next char
          nextChar(l);
          strBuff[dynLen] = l->curChar;
          ++dynLen;

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
        if (!text) {
          panic("OOM\n");
        }
        if (strcpy_s(text, (unsigned long long)(dynLen + 1), strBuff)) {
          panic("Couldn't copy string\n");
        }
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
        dynLen = 1;

        // Get the buffer ready
        memset(strBuff, 0, STR_BUFF_SIZE);
        strBuff[0] = l->curChar;

        // Get length of word
        while (VALID_IDENT_CHAR(l->peekChar) && dynLen < STR_BUFF_SIZE) {
          // Copy in next char
          nextChar(l);
          strBuff[dynLen] = l->curChar;
          ++dynLen;
        }

        // Save the token
        // NOTE: All of these keywords are known ahead of time, therefore no
        // dynamic allocation is needed
        if (!strcmp(strBuff, "break")) {
          token = (Token){NULL, T_BREAK, l->line};
        } else if (!strcmp(strBuff, "call")) {
          token = (Token){NULL, T_CALL, l->line};
        } else if (!strcmp(strBuff, "case")) {
          token = (Token){NULL, T_CASE, l->line};
        } else if (!strcmp(strBuff, "const")) {
          token = (Token){NULL, T_CONST, l->line};
        } else if (!strcmp(strBuff, "continue")) {
          token = (Token){NULL, T_CONTINUE, l->line};
        } else if (!strcmp(strBuff, "default")) {
          token = (Token){NULL, T_DEFAULT, l->line};
        } else if (!strcmp(strBuff, "elif")) {
          token = (Token){NULL, T_ELIF, l->line};
        } else if (!strcmp(strBuff, "else")) {
          token = (Token){NULL, T_ELSE, l->line};
        } else if (!strcmp(strBuff, "enum")) {
          token = (Token){NULL, T_ENUM, l->line};
        } else if (!strcmp(strBuff, "for")) {
          token = (Token){NULL, T_FOR, l->line};
        } else if (!strcmp(strBuff, "fun")) {
          token = (Token){NULL, T_FUN, l->line};
        } else if (!strcmp(strBuff, "if")) {
          token = (Token){NULL, T_IF, l->line};
        } else if (!strcmp(strBuff, "let")) {
          token = (Token){NULL, T_LET, l->line};
        } else if (!strcmp(strBuff, "make")) {
          token = (Token){NULL, T_MAKE, l->line};
        } else if (!strcmp(strBuff, "new")) {
          token = (Token){NULL, T_NEW, l->line};
        } else if (!strcmp(strBuff, "return")) {
          token = (Token){NULL, T_RETURN, l->line};
        } else if (!strcmp(strBuff, "struct")) {
          token = (Token){NULL, T_STRUCT, l->line};
        } else if (!strcmp(strBuff, "switch")) {
          token = (Token){NULL, T_SWITCH, l->line};
        } else if (!strcmp(strBuff, "nil")) {
          token = (Token){NULL, T_NIL, l->line};
        } else if (!strcmp(strBuff, "true")) {
          token = (Token){NULL, T_TRUE, l->line};
        } else if (!strcmp(strBuff, "false")) {
          token = (Token){NULL, T_FALSE, l->line};
        } else {
          // Copy over to dynamic allocation, freeing up strBuff for next char,
          // string, etc
          text = malloc((dynLen + 1) * sizeof(char));
          if (!text) {
            panic("OOM\n");
          }

          if (strcpy_s(text, STR_BUFF_SIZE, strBuff)) {
            panic("Couldn't copy string\n");
          }

          token = (Token){text, T_IDENTIFIER, l->line};
        }
      }

      // Bad char
      else {
        throwError(l, "Any other character", l->curChar);
      }
    }

    printf("%s\n", tokenCodeString(token.kind));
    // Add that token to the end of the list
    if (TokenListAppend(&l->out, token)) {
      panic("Couldn't append to token list.");
    }

    // Clear the token
    token = (Token){NULL, T_ILLEGAL, 0};

  NO_TOKEN:

    nextChar(l);
  }

  printf("Exit Lexer\n");
}
