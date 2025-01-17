#include "Analyser.h"
#include "Hoister.h"
#include "Lexer.h"
#include "Panic.h"
#include "Parser.h"
#include <stdbool.h>
#include <stdio.h>

bool validFileName(char fileName[]) {
  int i = 0;
  int dotIndex = -1;
  while (fileName[i]) {
    if (fileName[i] == '.') {
      dotIndex = i;
    }
    ++i;
  }

  // No file extension
  if (dotIndex == -1) {
    return false;
  }

  // Correct file extension length
  if (i - dotIndex != 4) {
    return false;
  }

  // Check extension
  if (fileName[dotIndex + 1] != 'n') {
    return false;
  }

  if (fileName[dotIndex + 2] != 'a') {
    return false;
  }

  if (fileName[dotIndex + 3] != 'v') {
    return false;
  }

  // It's good
  return true;
}

int main(int argc, char *argv[]) {
  // Check every file name
  printf("Validating files\n");
  for (int i = 1; i < argc; ++i) {
    if (!validFileName(argv[i])) {
      printf("Filename %s had incorrect file extension.\n", argv[i]);
      return 1;
    }
  }

  // Load all file pointers
  printf("Opening files\n");
  FILE *files[argc - 1];
  errno_t err;
  for (int i = 1; i < argc; ++i) {
    // Open the file
    err = fopen_s(&files[i - 1], argv[i], "r");
    if (err) {
      _fcloseall();
      printf("Couldn't open file %s\n", argv[i]);
      exit(1);
    }
  }
  printf("End validating files\n\n");

  // Lex every file
  printf("Lexing\n");
  Lexer lexers[argc - 1];
  for (int i = 1; i < argc; ++i) {
    lexerInit(&lexers[i - 1], argv[i], files[i - 1]);
    lex(&lexers[i - 1]);
  }

  // Print all lexed soruces
  // for (int i = 0; i < argc - 1; ++i) {
  //   for (int j = 0; j < lexers[i].out.len; ++j) {
  //     printf("%s, ", tokenString(&lexers[i].out.p[j]));
  //   }
  //   printf("\n");
  // }

  // Close all the files before moving on
  printf("Closing files\n");
  _fcloseall();
  printf("End lexing\n\n");

  // Parse every file
  printf("Parsing\n");

  Parser parsers[argc - 1];

  for (int i = 1; i < argc; ++i) {
    printf("Parsing %s\n", argv[i]);
    parserInit(&parsers[i - 1], argv[i], lexers[i - 1].out);
    parse(&parsers[i - 1]);

    // char *out = nodeString(&parsers[i - 1].out);
    // printf("%s\n", out);
    // free(out);
  }

  // Destroy the tokens
  printf("Destroying Lexer garbage\n");
  for (int i = 1; i < argc; ++i) {
    // for (int j = 0; j < lexers[i - 1].out.len; ++j) {
    //   tokenDestroy(&lexers[i - 1].out.p[j]);
    // }
    TokenListDestroy(&lexers[i - 1].out);
  }
  printf("End parsing\n\n");

  // Hoist from each file into one place
  printf("Hoisting\n");
  Hoister h;
  hoist(&h, parsers, argc - 1);
  printf("End hoisting\n\n");

  char *out;
  //
  // out = nodeString(&h.enums);
  // printf("Enums\n%s\n", out);
  // free(out);
  //
  // out = nodeString(&h.structs);
  // printf("Structs\n%s\n", out);
  // free(out);
  //
  // out = nodeString(&h.funcs);
  // printf("Funcs\n%s\n", out);
  // free(out);

  // Semantic Analysis
  printf("Analysing\n");
  Analyser a;
  analyserInit(&a, h.enums, h.structs, h.funcs);
  printf("Analyser initialised\n");
  analyse(&a);
  printf("End anlysis\n\n");

  // Optimize

  // Emit to C

  printf("Finished\n");

  return 0;
}
