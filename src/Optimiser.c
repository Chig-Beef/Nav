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

bool variableEliminationExpression(Optimiser *o, Node *block, VarList *vars) {
  bool changed = false;

  return changed;
}

bool variableEliminationBlock(Optimiser *o, Node *block, VarList *vars) {
  bool changed = false;

  if (vars == NULL) {
    if (VarListInit(vars, 1)) {
      panic("Couldn't init varlist\n");
    }
  }

  int varLen = vars->len;

  Node *assignment;
  int j;

  // For every statement
  for (int i = 0; i < block->children.len; ++i) {
    Node *n = block->children.p + i;

    switch (n->kind) {
    case N_VAR_DEC:
      assignment = n->children.p;
      if (assignment->kind == N_NEW_ASSIGNMENT) {
        // Check expression
        changed =
            changed |
            variableEliminationExpression(
                o, assignment->children.p + assignment->children.len - 1, vars);

        // Add new variable
        Variable v = {assignment->children.p[3].data, false, block, i};
        if (VarListAppend(vars, v)) {
          panic("Couldn't append to varlist\n");
        }

      } else if (assignment->children.p[0].kind != N_CREMENT) {
        // First check the index if there is one
        if (assignment->children.p[1].kind == N_INDEX) {
          Node *index = assignment->children.p + 1;
          changed = changed | variableEliminationExpression(
                                  o, index->children.p + 1, vars);
        }

        // Then check the expression
        changed =
            changed |
            variableEliminationExpression(
                o, assignment->children.p + assignment->children.len - 1, vars);
      } else { // Crement
               // Do nothing, doesn't count as using the variable
      }
      break;

    case N_LONE_CALL:
      // Check each arg
      Node *fn = n->children.p;
      for (j = 3; j < fn->children.len; j += 2) {
        variableEliminationExpression(o, fn->children.p + j, vars);
      }
      break;

    case N_FOR_LOOP:
      // Check first assignment, condition, second assignment, and block
      j = 1;
      assignment = n->children.p + j;

      if (assignment->kind == N_NEW_ASSIGNMENT) {
        // Check expression
        changed =
            changed |
            variableEliminationExpression(
                o, assignment->children.p + assignment->children.len - 1, vars);

        // Add new variable
        Variable v = {assignment->children.p[3].data, false, n, j};
        if (VarListAppend(vars, v)) {
          panic("Couldn't append to varlist\n");
        }
        j += 2;
      } else if (assignment->kind == N_ASSIGNMENT) {
        if (assignment->children.p[0].kind != N_CREMENT) {
          // First check the index if there is one
          if (assignment->children.p[1].kind == N_INDEX) {
            Node *index = assignment->children.p + 1;
            changed = changed | variableEliminationExpression(
                                    o, index->children.p + 1, vars);
          }

          // Then check the expression
          changed =
              changed |
              variableEliminationExpression(
                  o, assignment->children.p + assignment->children.len - 1,
                  vars);
        } else { // Crement
                 // Do nothing, doesn't count as using the variable
        }
        j += 2;
      } else {
        ++j;
      }

      Node *expr = n->children.p + j;
      if (expr->kind == N_EXPRESSION) {
        changed = changed | variableEliminationExpression(o, expr, vars);
        j += 2;
      } else {
        ++j;
      }

      assignment = n->children.p + j;
      if (assignment->kind == N_ASSIGNMENT) {
        if (assignment->children.p[0].kind != N_CREMENT) {
          // First check the index if there is one
          if (assignment->children.p[1].kind == N_INDEX) {
            Node *index = assignment->children.p + 1;
            changed = changed | variableEliminationExpression(
                                    o, index->children.p + 1, vars);
          }

          // Then check the expression
          changed =
              changed |
              variableEliminationExpression(
                  o, assignment->children.p + assignment->children.len - 1,
                  vars);
        } else { // Crement
                 // Do nothing, doesn't count as using the variable
        }
      }

      variableEliminationBlock(o, n->children.p + n->children.len - 1, vars);

      break;

    case N_IF_BLOCK:
      // Check condition and block, and repeat for each other case
      changed = changed | variableEliminationExpression(
                              o, assignment->children.p + 2, vars);

      changed = changed |
                variableEliminationBlock(o, assignment->children.p + 4, vars);

      j = 5;

      while (j < n->children.len) {
        if (n->children.p[j].kind == N_ELIF) {
          changed = changed | variableEliminationExpression(
                                  o, assignment->children.p + j + 2, vars);

          changed = changed | variableEliminationBlock(
                                  o, assignment->children.p + j + 4, vars);

          j += 5;
        } else { // Else
          changed = changed | variableEliminationBlock(
                                  o, assignment->children.p + j + 1, vars);
          break;
        }
      }
      break;

    case N_RET_STATE:
      if (assignment->children.p[1].kind == N_EXPRESSION) {
        changed = changed | variableEliminationExpression(
                                o, assignment->children.p + 1, vars);
      }
      break;

    case N_SWITCH_STATE:
      // Check item, and all cases + default.
      break;

    case N_BREAK_STATE:
      // Do nothing
      break;

    case N_CONTINUE_STATE:
      // Do nothing
      break;

    default:
      panic("Invalid statement in block");
    }
  }

  // Check for dead vars
  for (int i = varLen; i < vars->len; ++i) {
    Variable v = vars->p[i];
    if (v.used) {
      continue;
    }

    if (NodeListRemoveAt(&v.decBlock->children, v.decIndex)) {
      panic("Couldn't remove from nodelist\n");
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
