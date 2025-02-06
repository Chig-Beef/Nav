#pragma once

#include "Node.h"

typedef struct Emitter {
  // Source
  Node inEnums, inStructs, inFuns;

  // How indented the current position is
  int tabs;
} Emitter;

void emitterInit(Emitter *e, Node enums, Node structs, Node funs);

char *emit(Emitter *e);
