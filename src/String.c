#include "Panic.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct String {
  char *data;
  unsigned int arc;
  bool freeable;
} String;

// Attempts to free the string
void strFree(String *s) {
  if (s == NULL) {
    printf("Double free on string wrapper\n");
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
