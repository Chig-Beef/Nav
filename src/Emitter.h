#pragma once

#include "CharList.h"
#include "Node.h"
#include "StringManager.h"

typedef struct Emitter {
  // Source
  Node inEnums, inStructs, inFuns;

  // How indented the current position is
  int tabs;

  // NULL string
  char *nil;

  StringManager *sm;
} Emitter;

void emitterInit(Emitter *e, Node enums, Node structs, Node funs,
                 StringManager *sm);

CharList emit(Emitter *e);
