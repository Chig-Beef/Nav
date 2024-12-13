typedef struct Lexer {
  char *source;
  int len;
  int index;
  int line;
} Lexer;

char peekChar(Lexer *l) {
  if (l->index >= l->len - 1) {
    return 0;
  }

  return l->source[l->index + 1];
}
