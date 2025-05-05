#include "StringManager.h"

#include <corecrt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A megabyte dedicate to string storage
#define MAX_BULK 1000000

// Maximum hundred thousand unique strings
#define MAX_STRINGS 100000

errno_t initStringManager(StringManager *sm) {
  // Allocate bulk memory
  sm->bulk = calloc(MAX_BULK, sizeof(char));
  if (sm->bulk == NULL) {
    return 1;
  }

  // Allocate registry
  sm->registry = malloc(MAX_STRINGS * sizeof(char *));
  if (sm->registry == NULL) {
    return 1;
  }

  sm->regCount = 0;
  sm->regNext = sm->bulk;

  return 0;
}

void destroyStringManager(StringManager *sm) {
  free(sm->registry);
  free(sm->bulk);
}

// getString hard errors if no string can be returned.
// For this to occur, no match will be found.
// Furthermore, the string manager will have the inability to
// allocate a new string.
char *getString(StringManager *sm, const char *match) {
  // Check current registry
  int x = sm->regCount;
  // printf("Checking for match %s\n", match);

  for (int i = 0; i < sm->regCount; ++i) {
    printf("%s\n", sm->registry[i]);
    if (!strcmp(sm->registry[i], match)) {
      return sm->registry[i];
    }
  }

  // Couldn't find it, attempt to add it to the registry
  int matchLength = strlen(match);
  if (sm->regNext - sm->bulk + matchLength > MAX_BULK) {
    // Hard error, out of room
    printf(
        "Ran out of room to allocate for strings, attempting to allocate %s\n",
        match);
    exit(1);
    return NULL;
  }

  sm->registry[sm->regCount] = sm->regNext;
  ++sm->regCount;

  for (int i = 0; i < matchLength; ++i) {
    *sm->regNext = match[i];
    ++sm->regNext;
  }

  // Add that line terminator!
  *sm->regNext = 0;
  sm->regNext++;

  return sm->registry[sm->regCount - 1];
}

bool cmpStr(const char *a, const char *b) {
  if (a == b) {
    return true;
  }

  if (a == NULL || b == NULL) {
    return false;
  }

  return !strcmp(a, b);
}
