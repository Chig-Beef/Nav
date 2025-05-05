#pragma once

#include <corecrt.h>

typedef struct StringManager {
  // Keeps track of all string
  char **registry;
  int regCount;

  // Where all string data is stored
  char *bulk;
  char *regNext;
} StringManager;

errno_t initStringManager(StringManager *sm);

void destroyStringManager(StringManager *sm);

// getString hard errors if no string can be returned.
// For this to occur, no match will be found.
// Furthermore, the string manager will have the inability to
// allocate a new string.
char *getString(StringManager *sm, const char *match);
