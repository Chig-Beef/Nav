#include "Lexer.h"
#include "Panic.h"
#include "StringManager.h"
#include "Token.h"

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

void nextChar(Lexer *l) {
  l->curChar = l->peekChar;
  l->peekChar = fgetc(l->source);

  if (l->curChar == '\n') {
    ++l->line;
  }
}

void lexerInit(Lexer *l, char sourceName[], FILE *source, StringManager *sm) {
  l->sourceName = sourceName;
  l->source = source;
  l->line = 1;
  l->peekChar = 0;
  l->sm = sm;

  nextChar(l);
  nextChar(l);

  if (TokenListInit(&l->out, 8)) {
    panic("Couldn't initialise token list.");
  }
}

void throwLexerError(Lexer *l, char expected[], char got) {
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
      throwLexerError(l, "More source", 0);

    // Single character
    case '.':
      token = NEW_TOKEN(T_ACCESSOR);
      break;
    case ':':
      token = NEW_TOKEN(T_COLON);
      break;
    case ';':
      token = NEW_TOKEN(T_SEMICOLON);
      break;
    case ',':
      token = NEW_TOKEN(T_SEP);
      break;
    case '%':
      token = NEW_TOKEN(T_MOD);
      break;
    case '*':
      token = NEW_TOKEN(T_MUL);
      break;
    case '~':
      token = NEW_TOKEN(T_XOR);
      break;
    case '^':
      token = NEW_TOKEN(T_DEREF);
      break;
    case '`':
      token = NEW_TOKEN(T_REF);
      break;
    case '[':
      token = NEW_TOKEN(T_L_BLOCK);
      break;
    case ']':
      token = NEW_TOKEN(T_R_BLOCK);
      break;
    case '{':
      token = NEW_TOKEN(T_L_SQUIRLY);
      break;
    case '}':
      token = NEW_TOKEN(T_R_SQUIRLY);
      break;
    case '(':
      token = NEW_TOKEN(T_L_PAREN);
      break;
    case ')':
      token = NEW_TOKEN(T_R_PAREN);
      break;

      // Could be double
    case '=':
      if (l->peekChar == '=') {
        nextChar(l);
        token = NEW_TOKEN(T_EQ);
      } else {
        token = NEW_TOKEN(T_ASSIGN);
      }
      break;
    case '+':
      if (l->peekChar == '+') {
        nextChar(l);
        token = NEW_TOKEN(T_INC);
      } else {
        token = NEW_TOKEN(T_ADD);
      }
      break;
    case '&':
      if (l->peekChar == '&') {
        nextChar(l);
        token = NEW_TOKEN(T_ANDAND);
      } else {
        token = NEW_TOKEN(T_AND);
      }
      break;
    case '!':
      if (l->peekChar == '=') {
        nextChar(l);
        token = NEW_TOKEN(T_NEQ);
      } else {
        token = NEW_TOKEN(T_NOT);
      }
      break;
    case '|':
      if (l->peekChar == '|') {
        nextChar(l);
        token = NEW_TOKEN(T_OROR);
      } else {
        token = NEW_TOKEN(T_OR);
      }
      break;
    case '-':
      if (l->peekChar == '-') {
        nextChar(l);
        token = NEW_TOKEN(T_DEC);
      } else if (l->peekChar == '>') {
        nextChar(l);
        token = NEW_TOKEN(T_P_ACCESSOR);
      } else {
        token = NEW_TOKEN(T_SUB);
      }
      break;

      // Slightly larger cases
    case '>':
      if (l->peekChar == '>') {
        nextChar(l);
        token = NEW_TOKEN(T_R_SHIFT);
      } else if (l->peekChar == '=') {
        nextChar(l);
        token = NEW_TOKEN(T_GTEQ);
      } else {
        token = NEW_TOKEN(T_GT);
      }
      break;
    case '<':
      if (l->peekChar == '<') {
        nextChar(l);
        token = NEW_TOKEN(T_L_SHIFT);
      } else if (l->peekChar == '=') {
        nextChar(l);
        token = NEW_TOKEN(T_LTEQ);
      } else {
        token = NEW_TOKEN(T_LT);
      }
      break;

      // Comment, divide
    case '/':
      if (l->peekChar == '/') {
        while (l->curChar != '\n' && l->curChar >= 0) {
          nextChar(l);
        }
        // Now the character for the next loop is the first one of
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
        token = NEW_TOKEN(T_DIV);
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
        throwLexerError(l, "Char in character definition", '\'');
      case '\n':
        throwLexerError(l, "No newline in character", '\n');
      case '\r':
        throwLexerError(l, "No return in character", '\r');
      case '\\':
        strBuff[dynLen] = l->curChar;
        nextChar(l);
        ++dynLen;

        switch (l->curChar) {
        case '\n':
          throwLexerError(l, "No newline in character", '\n');
        case '\r':
          throwLexerError(l, "No return in character", '\r');
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
        throwLexerError(l, "End of character literal", l->curChar);
      }

      strBuff[dynLen] = '\'';
      ++dynLen;

      // Register new string, adding to token
      // Save the token
      token = (Token){getString(l->sm, strBuff), T_CHAR, l->line};
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
          throwLexerError(l, "No newline in string", '\n');
        case '\r':
          throwLexerError(l, "No return in string", '\r');
        case '\\':
          escaped = true;
        }

        // Copy in next char
        strBuff[dynLen] = l->curChar;
        nextChar(l);
        ++dynLen;

        // End of source?
        if (l->curChar < 0) {
          throwLexerError(l, "More source for string", 0);
        }
      }

      strBuff[dynLen] = l->curChar;
      ++dynLen;

      // Save the token
      token = (Token){getString(l->sm, strBuff), T_STRING, l->line};
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
              throwLexerError(l, "Only one decimal in float", '.');
            }
            isFloat = true;
          }
        }

        if (strBuff[dynLen - 1] == '.') {
          throwLexerError(l, "Digits after decimal", '.');
        }

        // Save the token
        if (isFloat) {
          token = (Token){getString(l->sm, strBuff), T_FLOAT, l->line};
        } else {
          token = (Token){getString(l->sm, strBuff), T_INT, l->line};
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
        if (cmpStr(strBuff, "break")) {
          token = NEW_TOKEN(T_BREAK);
        } else if (cmpStr(strBuff, "call")) {
          token = NEW_TOKEN(T_CALL);
        } else if (cmpStr(strBuff, "case")) {
          token = NEW_TOKEN(T_CASE);
        } else if (cmpStr(strBuff, "const")) {
          token = NEW_TOKEN(T_CONST);
        } else if (cmpStr(strBuff, "continue")) {
          token = NEW_TOKEN(T_CONTINUE);
        } else if (cmpStr(strBuff, "default")) {
          token = NEW_TOKEN(T_DEFAULT);
        } else if (cmpStr(strBuff, "elif")) {
          token = NEW_TOKEN(T_ELIF);
        } else if (cmpStr(strBuff, "else")) {
          token = NEW_TOKEN(T_ELSE);
        } else if (cmpStr(strBuff, "enum")) {
          token = NEW_TOKEN(T_ENUM);
        } else if (cmpStr(strBuff, "for")) {
          token = NEW_TOKEN(T_FOR);
        } else if (cmpStr(strBuff, "fun")) {
          token = NEW_TOKEN(T_FUN);
        } else if (cmpStr(strBuff, "if")) {
          token = NEW_TOKEN(T_IF);
        } else if (cmpStr(strBuff, "let")) {
          token = NEW_TOKEN(T_LET);
        } else if (cmpStr(strBuff, "make")) {
          token = NEW_TOKEN(T_MAKE);
        } else if (cmpStr(strBuff, "new")) {
          token = NEW_TOKEN(T_NEW);
        } else if (cmpStr(strBuff, "return")) {
          token = NEW_TOKEN(T_RETURN);
        } else if (cmpStr(strBuff, "struct")) {
          token = NEW_TOKEN(T_STRUCT);
        } else if (cmpStr(strBuff, "switch")) {
          token = NEW_TOKEN(T_SWITCH);
        } else if (cmpStr(strBuff, "true")) {
          token = NEW_TOKEN(T_TRUE);
        } else if (cmpStr(strBuff, "false")) {
          token = NEW_TOKEN(T_FALSE);
        } else {
          token = (Token){getString(l->sm, strBuff), T_IDENTIFIER, l->line};
        }
      }

      // Bad char
      else {
        throwLexerError(l, "Any other character", l->curChar);
      }
    }

    // Add that token to the end of the list
    if (TokenListAppend(&l->out, token)) {
      panic("Couldn't append to token list.");
    }

    // Clear the token
    token = ZERO_TOKEN;

  NO_TOKEN:

    nextChar(l);
  }
}
