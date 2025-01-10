#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef struct String {
  char *data;
  unsigned int arc;
  bool freeable;
} String;

// Attempts to free the string
void strFree(String *s);

// Gets another ref to this string
String *strGet(String *s);

String strNew(char *data, bool freeable);

void strReroute(String *s, char *data, bool freeable);
