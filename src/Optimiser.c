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

bool variableEliminationBlock(Optimiser *o, Node *block, VarList *vars);

bool variableEliminationStatement(Optimiser *o, Node *state, Node *block, int i,
                                  VarList *vars) {
  bool changed = false;

  Node *assignment, *fn, *index, *expr;
  Variable v;
  int j;

  switch (state->kind) {
  case N_VAR_DEC:
    assignment = state->children.p;
    if (assignment->kind == N_NEW_ASSIGNMENT) {
      // Check expression
      changed =
          changed |
          variableEliminationExpression(
              o, assignment->children.p + assignment->children.len - 1, vars);

      // Add new variable
      v = (Variable){assignment->children.p[3].data, false, block, i};
      if (VarListAppend(vars, v)) {
        panic("Couldn't append to varlist\n");
      }

    } else if (assignment->children.p[0].kind != N_CREMENT) {
      // First check the index if there is one
      if (assignment->children.p[1].kind == N_INDEX) {
        index = assignment->children.p + 1;
        changed = changed |
                  variableEliminationExpression(o, index->children.p + 1, vars);
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
    fn = state->children.p;
    for (j = 3; j < fn->children.len; j += 2) {
      changed =
          changed | variableEliminationExpression(o, fn->children.p + j, vars);
    }
    break;

  case N_FOR_LOOP:
    // Check first assignment, condition, second assignment, and block
    j = 1;
    assignment = state->children.p + j;

    if (assignment->kind == N_NEW_ASSIGNMENT) {
      // Check expression
      changed =
          changed |
          variableEliminationExpression(
              o, assignment->children.p + assignment->children.len - 1, vars);

      // Add new variable
      v = (Variable){assignment->children.p[3].data, false, state, j};
      if (VarListAppend(vars, v)) {
        panic("Couldn't append to varlist\n");
      }
      j += 2;
    } else if (assignment->kind == N_ASSIGNMENT) {
      if (assignment->children.p[0].kind != N_CREMENT) {
        // First check the index if there is one
        if (assignment->children.p[1].kind == N_INDEX) {
          index = assignment->children.p + 1;
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
      j += 2;
    } else {
      ++j;
    }

    expr = state->children.p + j;
    if (expr->kind == N_EXPRESSION) {
      changed = changed | variableEliminationExpression(o, expr, vars);
      j += 2;
    } else {
      ++j;
    }

    assignment = state->children.p + j;
    if (assignment->kind == N_ASSIGNMENT) {
      if (assignment->children.p[0].kind != N_CREMENT) {
        // First check the index if there is one
        if (assignment->children.p[1].kind == N_INDEX) {
          index = assignment->children.p + 1;
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
    }

    changed =
        changed | variableEliminationBlock(
                      o, state->children.p + state->children.len - 1, vars);

    break;

  case N_IF_BLOCK:
    // Check condition and block, and repeat for each other case
    changed =
        changed | variableEliminationExpression(o, state->children.p + 2, vars);

    changed =
        changed | variableEliminationBlock(o, state->children.p + 4, vars);

    j = 5;

    while (j < state->children.len) {
      if (state->children.p[j].kind == N_ELIF) {
        changed = changed | variableEliminationExpression(
                                o, state->children.p + j + 2, vars);

        changed = changed |
                  variableEliminationBlock(o, state->children.p + j + 4, vars);

        j += 5;
      } else { // Else
        changed = changed |
                  variableEliminationBlock(o, state->children.p + j + 1, vars);
        break;
      }
    }
    break;

  case N_RET_STATE:
    if (assignment->children.p[1].kind == N_EXPRESSION) {
      changed = changed |
                variableEliminationExpression(o, state->children.p + 1, vars);
    }
    break;

  case N_SWITCH_STATE:
    // Check item, and all cases + default.
    changed =
        changed | variableEliminationExpression(o, state->children.p + 2, vars);

    j = 5;

    // NOTE: Calling 'variableEliminationBlock' with caseBlock and default block
    // is fine

    Node *caseBlock = state->children.p + j;
    while (caseBlock->kind == N_CASE_BLOCK) {
      changed = changed | variableEliminationExpression(
                              o, caseBlock->children.p + 1, vars);

      int varLen = vars->len;

      // For every statement
      for (j = 3; j < caseBlock->children.len; ++j) {
        Node *state = caseBlock->children.p + j;
        changed = changed |
                  variableEliminationStatement(o, state, caseBlock, j, vars);
      }

      // Cleanup vars
      while (vars->len > varLen) {
        j = varLen - 1;
        v = vars->p[j];

        // Remove from code
        if (!v.used) {
          if (NodeListRemoveAt(&v.decBlock->children, v.decIndex)) {
            panic("Couldn't remove from nodelist\n");
          }
        }

        // Remove from vars
        if (VarListRemoveAt(vars, vars->len - 1)) {
          panic("Couldn't remove from list\n");
        }
      }

      ++j;
      caseBlock = state->children.p + j;
    }

    if (caseBlock->kind == N_DEFAULT_BLOCK) {
      int varLen = vars->len;

      // For every statement
      for (j = 2; j < caseBlock->children.len; ++j) {
        Node *state = caseBlock->children.p + j;
        changed = changed |
                  variableEliminationStatement(o, state, caseBlock, j, vars);
      }

      // Cleanup vars
      while (vars->len > varLen) {
        j = varLen - 1;
        v = vars->p[j];

        // Remove from code
        if (!v.used) {
          if (NodeListRemoveAt(&v.decBlock->children, v.decIndex)) {
            panic("Couldn't remove from nodelist\n");
          }
        }

        // Remove from vars
        if (VarListRemoveAt(vars, vars->len - 1)) {
          panic("Couldn't remove from list\n");
        }
      }
    }

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

  // For every statement
  for (int i = 0; i < block->children.len; ++i) {
    Node *state = block->children.p + i;
    changed = changed | variableEliminationStatement(o, state, block, i, vars);
  }

  // Cleanup vars
  while (vars->len > varLen) {
    int i = varLen - 1;
    Variable v = vars->p[i];

    // Remove from code
    if (!v.used) {
      if (NodeListRemoveAt(&v.decBlock->children, v.decIndex)) {
        panic("Couldn't remove from nodelist\n");
      }
    }

    // Remove from vars
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
