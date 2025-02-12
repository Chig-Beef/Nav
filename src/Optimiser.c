#include "Optimiser.h"
#include "Node.h"
#include "Panic.h"
#include "String.h"
#include "list.h"

void initOptimiser(Optimiser *o, Node funs) { o->src = funs; }

// NOTE: All optimisation methods return whether they produced a change, if so,
// then further optimisation could take place from earlier steps, and therefore
// the optimiser should go back to the start and optimise again

typedef struct Variable {
  String *name;
  bool used;
  Node *decBlock;
  int decIndex;
} Variable;

NEW_LIST_TYPE(Variable, Var)

#define SET_VAR_CHANGED                                                        \
  for (int k = 0; k < vars->len; ++k) {                                        \
    if (strEql(varName, vars->p[k].name)) {                                    \
      vars->p[k].used = true;                                                  \
      break;                                                                   \
    }                                                                          \
  }

bool variableEliminationBlock(Optimiser *o, Node *block, VarList *vars) {
  bool changed = false;

  if (vars == NULL) {
    if (VarListInit(vars, 1)) {
      panic("Couldn't init varlist\n");
    }
  }

  int varLen = vars->len;

  // For every statement
  for (int i = 0; i < block->children.len; ++i) {
    Node *n = block->children.p + i;

    switch (n->kind) {
    case N_LONE_CALL:
      break;

    case N_VAR_DEC:
      Node *assignment = n->children.p;
      if (assignment->kind == N_NEW_ASSIGNMENT) {
        Variable v = {assignment->children.p[3].data, false, n, i};
        if (VarListAppend(vars, v)) {
          panic("Couldn't append to varlist\n");
        }

      } else if (assignment->children.p[0].kind == N_CREMENT) {
        assignment = assignment->children.p;

        String *varName;

        if (assignment->children.p[1].kind == N_IDENTIFIER) {
          varName = assignment->children.p[1].data;
        } else { // Access
          varName = assignment->children.p[1].children.p[0].data;
        }

        SET_VAR_CHANGED

      } else { // Regular assignment

        String *varName;

        if (assignment->children.p[0].kind == N_IDENTIFIER) {
          varName = assignment->children.p[0].data;
        } else { // Access
          varName = assignment->children.p[0].children.p[0].data;
        }

        SET_VAR_CHANGED
      }
      break;

    case N_IF_BLOCK:
      break;

    case N_FOR_LOOP:
      break;

    case N_RET_STATE:
      break;

    case N_BREAK_STATE:
      break;

    case N_CONTINUE_STATE:
      break;

    case N_SWITCH_STATE:
      break;

    default:
      panic("Invalid statement in block");
    }
  }

  // Cleanup vars
  while (vars->len > varLen) {
    if (VarListRemoveAt(vars, vars->len - 1)) {
      panic("Couldn't remove from list\n");
    }
  }

  return changed;
}

bool variableElimination(Optimiser *o) {
  bool changed = false;

  // For every function
  for (int i = 0; i < o->src.children.len; ++i) {
    Node *fn = o->src.children.p + i;

    // Optimise the block
    if (variableEliminationBlock(o, fn->children.p + fn->children.len - 1,
                                 NULL)) {
      changed = true;
    }
  }

  return changed;
}

bool branchElimination(Optimiser *o) { return false; }

bool constantFolding(Optimiser *o) { return false; }

bool strengthReduction(Optimiser *o) { return false; }

void optimise(Optimiser *o) {
RETRY_OPTIMISATION:

  // First step doesn't need to try again if a change is produced
  variableElimination(o);

  if (branchElimination(o)) {
    goto RETRY_OPTIMISATION;
  }

  if (constantFolding(o)) {
    goto RETRY_OPTIMISATION;
  }

  if (strengthReduction(o)) {
    goto RETRY_OPTIMISATION;
  }
}
