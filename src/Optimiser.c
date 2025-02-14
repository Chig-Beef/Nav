#include "Optimiser.h"
#include "Node.h"
#include "Panic.h"
#include "String.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

void optimiserInit(Optimiser *o, Node funs) { o->src = funs; }

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

#define NODE_LIST_REMOVE(list, index)                                          \
  if (NodeListRemoveAt((list), (index))) {                                     \
    panic("Couldn't remove from nodelist\n");                                  \
  }

void scrubVariable(Optimiser *o, String *name, Node *n, Node *parent,
                   int index) {
  Node *assignment;
  Node *ident;
  String *other;

  switch (n->kind) {
  case N_VAR_DEC:
    assignment = n->children.p;

    // Don't even look at new assignment
    if (assignment->kind == N_ASSIGNMENT) {
      if (assignment->children.p[0].kind == N_CREMENT) {
        assignment = assignment->children.p;
        ident = assignment->children.p + 1;
        if (ident->kind == N_ACCESS) {
          ident = ident->children.p;
        }
        other = ident->data;

        if (strEql(name, other)) {
          NODE_LIST_REMOVE(&parent->children, index)
        }
      } else {
        ident = assignment->children.p;
        if (ident->kind == N_ACCESS) {
          ident = ident->children.p;
        }
        other = ident->data;

        if (strEql(name, other)) {
          NODE_LIST_REMOVE(&parent->children, index)
        }
      }
    }
    break;

  case N_FOR_LOOP:
    if (n->children.p[2].kind == N_ASSIGNMENT) {
      scrubVariable(o, name, n->children.p + 2, n, 2);
    }

    if (n->children.p[n->children.len - 3].kind == N_ASSIGNMENT) {
      scrubVariable(o, name, n->children.p + n->children.len - 3, n,
                    n->children.len - 3);
    }

    scrubVariable(o, name, n->children.p + n->children.len - 1, n,
                  n->children.len - 1);
    break;

  case N_IF_BLOCK:
    scrubVariable(o, name, n->children.p + 4, n, 4);

    int i = 5;

    while (i < n->children.len) {
      if (n->children.p[i].kind == N_ELIF) {
        scrubVariable(o, name, n->children.p + i + 4, n, i + 4);
      } else {
        scrubVariable(o, name, n->children.p + i + 1, n, i + 1);
        break;
      }

      i += 5;
    }
    break;

  case N_SWITCH_STATE:
    for (int i = 5; i < n->children.len - 1; ++i) {
      scrubVariable(o, name, n->children.p + i, n, i);
    }

    break;

  case N_CASE_BLOCK:
    for (int i = 3; i < n->children.len; ++i) {
      scrubVariable(o, name, n->children.p + i, n, i);
    }
    break;

  case N_DEFAULT_BLOCK:
    for (int i = 2; i < n->children.len; ++i) {
      scrubVariable(o, name, n->children.p + i, n, i);
    }
    break;

  case N_BLOCK:
    for (int i = 1; i < n->children.len - 1; ++i) {
      scrubVariable(o, name, n->children.p + i, n, i);
    }
    break;

  default:
    // Do nothing
    break;
  }
}

void removeVariable(Optimiser *o, Variable v) {
  printf("Removing variable %s\n", v.name->data);
  scrubVariable(o, v.name, v.decBlock, NULL, 0);

  // Remove the original declaration
  NODE_LIST_REMOVE(&v.decBlock->children, v.decIndex)
}

void variableEliminationExpression(Optimiser *o, Node *expr, VarList *vars);

void variableEliminationValue(Optimiser *o, Node *val, VarList *vars) {
  switch (val->kind) {
  case N_UNARY_VALUE: // If we're actually a unary value, check the actual value
                      // inside
    if (val->children.p[0].kind == N_INDEX) {
      // Also check inside of the index
      variableEliminationExpression(o, val->children.p[0].children.p + 1, vars);
    }
    variableEliminationValue(o, val->children.p + 1, vars);
    return;

  case N_BRACKETED_VALUE:
    variableEliminationExpression(o, val->children.p + 1, vars);
    return;

  case N_FUNC_CALL:
    for (int i = 3; i < val->children.len - 1; i += 2) {
      variableEliminationExpression(o, val->children.p + i, vars);
    }
    return;

  case N_MAKE_ARRAY:
    for (int i = 2; i < val->children.len - 1; i += 2) {
      variableEliminationExpression(o, val->children.p + i, vars);
    }
    return;

  case N_STRUCT_NEW:
    for (int i = 3; i < val->children.len - 1; i += 2) {
      variableEliminationExpression(o, val->children.p + i, vars);
    }
    return;

  case N_ACCESS:
    variableEliminationValue(o, val->children.p, vars);
    return;

  case N_IDENTIFIER:
    String *name = val->data;

    for (int i = 0; i < vars->len; ++i) {
      Variable *v = vars->p + i;

      if (strEql(name, v->name)) {
        v->used = true;
      }
    }
    return;

  default:
    return;
  }
}

void variableEliminationExpression(Optimiser *o, Node *expr, VarList *vars) {
  // printf("Eliminating variables (expression) %s\n",
  // nodeCodeString(expr->kind));

  // NOTE: Due to precedence implementation there are 2 forms of expression,
  // either a single value, or 2 values. Also note these values can be unary
  // values

  // Check first operand
  variableEliminationValue(o, expr->children.p, vars);

  // Check second operand (if any)
  if (expr->children.len == 3) {
    variableEliminationValue(o, expr->children.p + 2, vars);
  }
}

bool variableEliminationBlock(Optimiser *o, Node *block, VarList *vars);

bool variableEliminationStatement(Optimiser *o, Node *state, Node *block, int i,
                                  VarList *vars) {
  // printf("Eliminating variables (statement) %s\n",
  // nodeCodeString(state->kind));
  bool changed = false;

  Node *assignment, *fn, *index, *expr;
  Variable v;
  int j;

  switch (state->kind) {
  case N_VAR_DEC:
    assignment = state->children.p;
    if (assignment->kind == N_NEW_ASSIGNMENT) {
      // Check expression
      variableEliminationExpression(
          o, assignment->children.p + assignment->children.len - 1, vars);

      // Add new variable
      v = (Variable){assignment->children.p[2].data, false, block, i};
      if (VarListAppend(vars, v)) {
        panic("Couldn't append to varlist\n");
      }

    } else if (assignment->children.p[0].kind != N_CREMENT) {
      // First check the index if there is one
      if (assignment->children.p[1].kind == N_INDEX) {
        index = assignment->children.p + 1;

        variableEliminationExpression(o, index->children.p + 1, vars);
      }

      // Then check the expression
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
      variableEliminationExpression(o, fn->children.p + j, vars);
    }
    break;

  case N_FOR_LOOP:
    // Check first assignment, condition, second assignment, and block
    j = 2;
    assignment = state->children.p + j;

    if (assignment->kind == N_NEW_ASSIGNMENT) {
      // Check expression
      variableEliminationExpression(
          o, assignment->children.p + assignment->children.len - 1, vars);

      // Add new variable
      v = (Variable){assignment->children.p[2].data, false, state, j};
      if (VarListAppend(vars, v)) {
        panic("Couldn't append to varlist\n");
      }
      j += 2;
    } else if (assignment->kind == N_ASSIGNMENT) {
      if (assignment->children.p[0].kind != N_CREMENT) {
        // First check the index if there is one
        if (assignment->children.p[1].kind == N_INDEX) {
          index = assignment->children.p + 1;

          variableEliminationExpression(o, index->children.p + 1, vars);
        }

        // Then check the expression
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
      variableEliminationExpression(o, expr, vars);
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

          variableEliminationExpression(o, index->children.p + 1, vars);
        }

        // Then check the expression
        variableEliminationExpression(
            o, assignment->children.p + assignment->children.len - 1, vars);
      } else { // Crement
               // Do nothing, doesn't count as using the variable
      }
    }

    changed |= variableEliminationBlock(
        o, state->children.p + state->children.len - 1, vars);

    break;

  case N_IF_BLOCK:
    // Check condition and block, and repeat for each other case
    variableEliminationExpression(o, state->children.p + 2, vars);

    changed |= variableEliminationBlock(o, state->children.p + 4, vars);

    j = 5;

    while (j < state->children.len) {
      if (state->children.p[j].kind == N_ELIF) {

        variableEliminationExpression(o, state->children.p + j + 2, vars);

        changed |= variableEliminationBlock(o, state->children.p + j + 4, vars);

        j += 5;
      } else { // Else
        changed |= variableEliminationBlock(o, state->children.p + j + 1, vars);
        break;
      }
    }
    break;

  case N_RET_STATE:
    if (state->children.p[1].kind == N_EXPRESSION) {
      variableEliminationExpression(o, state->children.p + 1, vars);
    }
    break;

  case N_SWITCH_STATE:
    // Check item, and all cases + default.
    variableEliminationExpression(o, state->children.p + 2, vars);

    j = 5;

    // NOTE: Calling 'variableEliminationBlock' with caseBlock and default block
    // is fine

    Node *caseBlock = state->children.p + j;
    while (caseBlock->kind == N_CASE_BLOCK) {

      variableEliminationExpression(o, caseBlock->children.p + 1, vars);

      int varLen = vars->len;

      // For every statement
      for (j = 3; j < caseBlock->children.len; ++j) {
        Node *state = caseBlock->children.p + j;
        changed |= variableEliminationStatement(o, state, caseBlock, j, vars);
      }

      // Cleanup vars
      while (vars->len > varLen) {
        j = varLen - 1;
        v = vars->p[j];

        // Remove from code
        if (!v.used) {
          changed = true;
          removeVariable(o, v);
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
        changed |= variableEliminationStatement(o, state, caseBlock, j, vars);
      }

      // Cleanup vars
      while (vars->len > varLen) {
        j = varLen - 1;
        v = vars->p[j];

        // Remove from code
        if (!v.used) {
          removeVariable(o, v);
          changed = true;
        }

        // Remove from vars
        if (VarListRemoveAt(vars, vars->len - 1)) {
          panic("Couldn't remove from list\n");
        }
      }
    }

    break;

  default:
    break;
  }

  return changed;
}

bool variableEliminationBlock(Optimiser *o, Node *block, VarList *vars) {
  // printf("Eliminating variables (block) %s\n", nodeCodeString(block->kind));

  bool changed = false;
  int varLen = vars->len;

  // For every statement
  for (int i = 1; i < block->children.len - 1; ++i) {
    Node *state = block->children.p + i;
    changed |= variableEliminationStatement(o, state, block, i, vars);
  }

  // Cleanup vars
  while (vars->len > varLen) {

    int i = vars->len - 1;
    Variable v = vars->p[i];

    // Remove from code
    if (!v.used) {
      removeVariable(o, v);
      changed = true;
    }

    // Remove from vars
    if (VarListRemoveAt(vars, vars->len - 1)) {
      panic("Couldn't remove from list\n");
    }
  }

  return changed;
}

bool variableElimination(Optimiser *o) {
  // printf("Eliminating variables\n");
  bool changed = false;

  VarList vars;
  if (VarListInit(&vars, 1)) {
    panic("Couldn't init varlist\n");
  }

  // For every function
  for (int i = 0; i < o->src.children.len; ++i) {
    Node *fn = o->src.children.p + i;

    // Optimise the block
    changed |= variableEliminationBlock(
        o, fn->children.p + fn->children.len - 1, &vars);
  }

  return changed;
}

union ConstVal {
  char c;
  float f;
  int i;
  bool b;
  char *s;
};

typedef struct Const {
  NodeCode type;
  union ConstVal val;
  String *data;
} Const;

Const getConstFromValue(Optimiser *o, Node *val) {
  Const out = {N_ILLEGAL};

  switch (val->kind) {
  case N_CHAR:
    // TODO: Implement getting the value from the char
    break;
  case N_FLOAT:
    // TODO: Implement getting the value from the float
    break;
  case N_INT:
    out.type = N_INT;
    out.val.i = atoi(val->data->data);
    break;
  case N_STRING:
    out.type = N_STRING;
    out.val.s = val->data->data;
    break;
  case N_TRUE:
    out.type = N_TRUE;
    out.val.b = true;
    break;
  case N_FALSE:
    out.type = N_TRUE;
    out.val.b = false;
    break;
  default:
    break;
  }

  out.data = val->data;

  return out;
}

Const getConstFromExpr(Optimiser *o, Node *expr) {
  if (expr->children.len != 1) {
    return (Const){N_ILLEGAL};
  }

  Node *val = expr->children.p;
  return getConstFromValue(o, val);
}

bool branchEliminationBlock(Optimiser *o, Node *block);

bool branchEliminationStatement(Optimiser *o, Node *state, Node *block, int i) {
  // printf("Eliminating branches (statement) %s\n",
  // nodeCodeString(state->kind));
  bool changed = false;

  Node *recBlock;

  switch (state->kind) {
  case N_FOR_LOOP:
    // Empty block
    if (state->children.p[state->children.len - 1].children.len == 2) {
      changed = true;

      // Remove the loop
      NODE_LIST_REMOVE(&block->children, i)
    } else {
      // Check child block
      changed |= branchEliminationBlock(o, state->children.p +
                                               state->children.len - 1);
    }
    break;
  case N_IF_BLOCK:
    // NOTE: Since the next elifs and elses depend on the previous ones, we can
    // only delete the last one at a time

    // char *out = nodeString(state);
    // printf("If statement\n%s\n", out);
    // free(out);

    recBlock = state->children.p + state->children.len - 1;
    if (state->children.p[state->children.len - 2].kind == N_ELSE) {
      // Check child block
      changed |= branchEliminationBlock(o, recBlock);

      // Empty block
      if (recBlock->children.len == 2) {
        changed = true;
        printf("Removing else statement\n");

        // Remove the loop
        for (int j = 0; j < 2; j++) {
          NODE_LIST_REMOVE(&state->children, state->children.len - 1)
        }
      }
    } else if (state->children.p[state->children.len - 5].kind == N_ELIF) {
      // Check child block
      changed |= branchEliminationBlock(o, recBlock);

      // Empty block
      if (recBlock->children.len == 2) {
        changed = true;
        printf("Removing elif statement\n");

        // Remove the loop
        for (int j = 0; j < 5; j++) {
          NODE_LIST_REMOVE(&state->children, state->children.len - 1)
        }
      }
    } else { // Single if statement
      // Check child block
      changed |= branchEliminationBlock(o, recBlock);

      // Empty block
      if (recBlock->children.len == 2) {
        changed = true;
        printf("Removing empty if statement\n");

        // Remove the loop
        NODE_LIST_REMOVE(&block->children, i)
      } else {
        // Try to eliminate based on constant value
        Const c = getConstFromExpr(o, state->children.p + 2);

        bool isTrue = false;

        switch (c.type) {
        case N_CHAR:
          isTrue = c.val.c != 0;
          break;
        case N_FLOAT:
          isTrue = c.val.f != 0.0;
          break;
        case N_INT:
          isTrue = c.val.i != 0;
          break;
        case N_STRING:
          isTrue = c.val.s != NULL;
          break;
        case N_TRUE: // N_TRUE means bool
          isTrue = c.val.b;
          break;
        default:
          return changed;
        }

        if (isTrue) { // The branch always happens
          changed = true;
          printf("Removing true if statement\n");

          // Remove the loop
          NODE_LIST_REMOVE(&block->children, i)

          for (int j = 1; j < recBlock->children.len - 1; ++j) {
            if (NodeListInsertAt(&block->children, recBlock->children.p[j],
                                 i)) {
              panic("Couldn't insert in nodelist\n");
            }
          }

        } else { // The branch never happens
          changed = true;
          printf("Removing false if statement\n");

          // Remove the loop
          NODE_LIST_REMOVE(&block->children, i)
        }
      }
    }

    break;
  case N_SWITCH_STATE:
    break;
  default:
    break;
  }

  return changed;
}

bool branchEliminationBlock(Optimiser *o, Node *block) {
  // printf("Eliminating branch (block) %s\n", nodeCodeString(block->kind));

  bool changed = false;

  // For every statement
  for (int i = 1; i < block->children.len - 1; ++i) {
    Node *state = block->children.p + i;
    changed |= branchEliminationStatement(o, state, block, i);
  }

  return changed;
}

bool branchElimination(Optimiser *o) {
  // printf("Eliminating branches\n");
  bool changed = false;

  // For every function
  for (int i = 0; i < o->src.children.len; ++i) {
    Node *fn = o->src.children.p + i;

    // Optimise the block
    changed |= branchEliminationBlock(o, fn->children.p + fn->children.len - 1);
  }

  return changed;
}

bool expressionFold(Optimiser *o, Node *n);

bool valueFold(Optimiser *o, Node *n) {
  bool changed = false;

  switch (n->kind) {
  case N_MAKE_ARRAY:
    for (int i = 2; i < n->children.len - 1; i += 2) {
      changed |= expressionFold(o, n->children.p + i);
    }
    return changed;
  case N_FUNC_CALL:
    for (int i = 3; i < n->children.len - 1; i += 2) {
      changed |= expressionFold(o, n->children.p + i);
    }
    return changed;
  case N_STRUCT_NEW:
    for (int i = 2; i < n->children.len - 1; i += 2) {
      changed |= expressionFold(o, n->children.p + i);
    }
    return changed;
  case N_BRACKETED_VALUE:
    changed |= expressionFold(o, n->children.p + 1);

    // If it's a single value in the brackets, take it out
    Node *innerExpr = n->children.p + 1;
    if (innerExpr->children.len == 1) {
      *n = innerExpr->children.p[0];
    }
    return changed;

    // Can't compress
  default:
    return false;
  }
}

bool expressionFold(Optimiser *o, Node *n) {
  // printf("Expression folding %s\n", nodeCodeString(n->kind));

  // Only need to check one value
  if (n->children.len == 1) {
    return valueFold(o, n->children.p);
  }

  // Are we able to turn the 2 values into a single one?
  bool changed = false;

  // Fold both values in expression
  changed |= valueFold(o, n->children.p);
  changed |= valueFold(o, n->children.p + 2);

  // Get the constant value (if it is constant)
  Const left, right;
  left = getConstFromValue(o, n->children.p);
  right = getConstFromValue(o, n->children.p + 2);

  NodeCode op = n->children.p[1].kind;

  // Can we fold?
  if (left.type == N_ILLEGAL || right.type == N_ILLEGAL) {
    // Before we take this as a loss, we should check whether this expression is
    // comparing the same identifier
    if (n->children.p[0].kind != N_IDENTIFIER ||
        n->children.p[2].kind != N_IDENTIFIER) {
      return changed;
    }

    // Same name?
    if (!strEql(n->children.p[0].data, n->children.p[2].data)) {
      return changed;
    }

    Node final = n->children.p[0];

    // Attempt a replacement
    switch (op) {
    case N_AND: // Results in the same value
      break;
    case N_OR: // Results in the same value
      break;
    case N_DIV: // Results in 1
      final.kind = N_INT;
      final.data = strNew("1", false);
      break;
    case N_EQ: // Results in true
      final.kind = N_TRUE;
      break;
    case N_GT: // Results in false
      final.kind = N_FALSE;
      break;
    case N_GTEQ: // Results in true
      final.kind = N_TRUE;
      break;
    case N_LT: // Results in false
      final.kind = N_FALSE;
      break;
    case N_LTEQ: // Results in true
      final.kind = N_TRUE;
      break;
    case N_MOD: // Results in 0
      final.kind = N_INT;
      final.data = strNew("0", false);
      break;
    case N_NEQ: // Results in false
      final.kind = N_FALSE;
      break;
    case N_SUB: // Results in 0
      final.kind = N_INT;
      final.data = strNew("0", false);
      break;
    case N_XOR: // Results in 0
      final.kind = N_INT;
      final.data = strNew("0", false);
      break;
    default:
      return changed;
    }

    // Replace left with new node
    n->children.p[0] = final;

    // Delete the op and right
    NODE_LIST_REMOVE(&n->children, 1)
    NODE_LIST_REMOVE(&n->children, 1)

    return changed;
  }

  // Are they even the same type?
  if (left.type != right.type) {
    panic("Incorrect typing in expression");
  }

  bool bv;
  int iv;
  char *v;
  String *final;

  switch (left.type) {
  case N_INT:
    switch (op) {
    case N_ADD:
      iv = left.val.i + right.val.i;
      break;
    case N_AND:
      iv = left.val.i & right.val.i;
      break;
    case N_ANDAND:
      iv = left.val.i && right.val.i;
      break;
    case N_DIV:
      iv = left.val.i / right.val.i;
      break;
    case N_EQ:
      // NOTE: This means 7 == 7 doesn't produce true, it produces 1
      iv = left.val.i == right.val.i;
      break;
    case N_GT:
      iv = left.val.i > right.val.i;
      break;
    case N_GTEQ:
      iv = left.val.i >= right.val.i;
      break;
    case N_LT:
      iv = left.val.i < right.val.i;
      break;
    case N_LTEQ:
      iv = left.val.i <= right.val.i;
      break;
    case N_MOD:
      iv = left.val.i % right.val.i;
      break;
    case N_MUL:
      iv = left.val.i * right.val.i;
      break;
    case N_NEQ:
      iv = left.val.i != right.val.i;
      break;
    case N_OR:
      iv = left.val.i | right.val.i;
      break;
    case N_OROR:
      iv = left.val.i || right.val.i;
      break;
    case N_SUB:
      iv = left.val.i - right.val.i;
      break;
    case N_XOR:
      iv = left.val.i ^ right.val.i;
      break;
    case N_L_SHIFT:
      iv = left.val.i << right.val.i;
      break;
    case N_R_SHIFT:
      iv = left.val.i >> right.val.i;
      break;
    default:
      panic("Invalid operator\n");
    }

    // Now we have the value in iv, we have to rearrange the expression to add
    // it in
    v = malloc(20);
    if (_itoa_s(iv, v, 20, 10)) {
      panic("Couldn't str number\n");
    }

    // Convert into string
    final = strNew(v, true);

    // Replace left with new node
    n->children.p[0].data = final;

    // Delete the op and right
    NODE_LIST_REMOVE(&n->children, 1)
    NODE_LIST_REMOVE(&n->children, 1)
    break;
  case N_TRUE:
    // TODO: Implement folding booleans
    return changed;
  case N_CHAR:
    // TODO: Implement getting the value from the char
    return changed;
  case N_FLOAT:
    // TODO: Implement getting the value from the float
    return changed;
  case N_STRING:
    // TODO: Implement folding strings
    return changed;
  default:
    return changed;
  }

  changed = true;

  return changed;
}

// This function simply recurses down the AST until it finds an expression to
// fold
bool expressionFoldingRec(Optimiser *o, Node *n) {
  // printf("Recursing to expression %s\n", nodeCodeString(n->kind));

  // Finally found an expression
  if (n->kind == N_EXPRESSION) {
    return expressionFold(o, n);
  }

  // Otherwise keep looking

  bool changed = false;

  // TODO: Optimise by only recursing to needed nodes
  for (int i = 0; i < n->children.len; ++i) {
    changed |= expressionFoldingRec(o, n->children.p + i);
  }

  return changed;
}

typedef struct Stopper {
  bool changed;
  bool stop;
} Stopper;

Stopper constantPropogateRec(Optimiser *o, Node *n, String *name, Const c) {
  // printf("Propogating to %s\n", nodeCodeString(n->kind));

  switch (n->kind) {
  case N_CREMENT:
    if (strEql(name, n->children.p[1].data)) {
      return (Stopper){false, true};
    }
    break;
  case N_ASSIGNMENT:
    if (strEql(name, n->children.p[0].data)) {
      return (Stopper){false, true};
    }
    break;
  case N_IDENTIFIER:
    if (strEql(name, n->data)) {
      n->data = c.data;
      n->kind = c.type;
    }
    return (Stopper){true, false};
  default:
    break;
  }

  Stopper changed = {false, false};
  Stopper s;

  // Recurse
  for (int i = 0; i < n->children.len; ++i) {
    s = constantPropogateRec(o, n->children.p + i, name, c);
    changed.changed |= s.changed;
    if (s.stop) {
      changed.stop = true;
      return changed;
    }
  }

  return changed;
}

bool constantPropogation(Optimiser *o, Node *n, Node *parent, int i) {
  // printf("Propogating variable dec %s\n", nodeCodeString(n->kind));

  Node *assignment = n->children.p;

  // We're looking for the original definition
  if (assignment->kind != N_NEW_ASSIGNMENT) {
    return false;
  }

  // Get the expression that is used for this assignment
  Node *expr = assignment->children.p + assignment->children.len - 1;
  Const c = getConstFromExpr(o, expr);

  // Is it constant?
  if (c.type == N_ILLEGAL) {
    return false;
  }

  // Now we have a constant value we can start propogating
  bool changed = false;

  // Keep a reference to the name of the variable
  String *name = assignment->children.p[2].data;

  for (int j = i + 1; j < parent->children.len; ++j) {
    if (constantPropogateRec(o, parent->children.p + j, name, c).stop) {
      break;
    }
  }

  // TODO: Recursively go through everything, replacing all references until a
  // new assignment is reached

  // TODO: If the new assignment is constant, continue with new value

  return changed;
}

bool constantPropogationRec(Optimiser *o, Node *n, Node *parent, int i) {
  // printf("Recursing to var dec %s\n", nodeCodeString(n->kind));

  // Finally found a variable declaration
  if (n->kind == N_VAR_DEC) {
    return constantPropogation(o, n, parent, i);
  }

  // Otherwise keep looking

  bool changed = false;

  // TODO: Optimise by only recursing to needed nodes
  for (int i = 0; i < n->children.len; ++i) {
    changed |= constantPropogationRec(o, n->children.p + i, n, i);
  }

  return changed;
}

bool constantFolding(Optimiser *o) {
  bool changed = expressionFoldingRec(o, &o->src);
  return changed | constantPropogationRec(o, &o->src, NULL, 0);
}

bool strengthReduction(Optimiser *o) { return false; }

void optimise(Optimiser *o) {
RETRY_OPTIMISATION:

  if (variableElimination(o)) {
    goto RETRY_OPTIMISATION;
  }

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
