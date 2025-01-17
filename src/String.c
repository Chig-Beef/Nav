#include "String.h"
#include "Panic.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool strEql(String *s1, String *s2) {
  // Same string
  if (s1 == s2) {
    return true;
  }

  // One string doesn't exist
  if (s1 == NULL || s2 == NULL) {
    return false;
  }

  // Both strings point to same underlying array
  if (s1->data == s2->data) {
    return true;
  }

  // A string points to nothing
  if (s1->data == NULL || s2->data == NULL) {
    return false;
  }

  char *p1 = s1->data;
  char *p2 = s2->data;

  while (*p1 && *p2) {
    if (*p1 != *p2) {
      return false;
    }

    ++p1;
    ++p2;
  }

  if (*p1 != *p2) {
    return false;
  }

  return true;
}

// Attempts to free the string
void strFree(String *s) {
  if (s == NULL) {
    return;
  }

  --s->arc;

  if (s->arc == 0 && s->freeable) {
    free(s->data);

    // Also rid self
    free(s);
  }

  s->freeable = false;
}

// Gets another ref to this string
String *strGet(String *s) {
  ++s->arc;
  return s;
}

String *strNew(char *data, bool freeable) {
  String *string = malloc(sizeof(String));

  if (string == NULL) {
    panic("Couldn't allocate string wrapper\n");
  }

  string->freeable = freeable;
  string->data = data;
  string->arc = 1;

  return string;
}

// Free old data, and put in new
void strReroute(String *s, char *data, bool freeable) {
  if (s->freeable) {
    free(s->data);
  }

  s->data = data;
  s->freeable = freeable;
}
