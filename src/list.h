#pragma once

#include <corecrt.h>
#include <stdlib.h>

#define ZERO_LIST (NodeList){NULL, 0, 0}

// TODO:
// pop, insert, clear, slice, sliceEnd, join, splice?

#define NEW_LIST_TYPE_HEADER(T, TYPE_NAME)                                     \
  typedef struct TYPE_NAME##List {                                             \
    T *p;                                                                      \
    int len;                                                                   \
    int cap;                                                                   \
  } TYPE_NAME##List;                                                           \
  errno_t TYPE_NAME##ListInit(TYPE_NAME##List *l, int initialSize);            \
  void TYPE_NAME##ListDestroy(TYPE_NAME##List *l);                             \
  errno_t TYPE_NAME##ListAppend(TYPE_NAME##List *l, T item);                   \
  TYPE_NAME##List TYPE_NAME##ListCopy(TYPE_NAME##List *src);                   \
  errno_t TYPE_NAME##ListRemoveAt(TYPE_NAME##List *l, int index);              \
  errno_t TYPE_NAME##ListInsertAt(TYPE_NAME##List *l, T item, int index);

#define NEW_LIST_TYPE(T, TYPE_NAME)                                            \
  typedef struct TYPE_NAME##List {                                             \
    T *p;                                                                      \
    int len;                                                                   \
    int cap;                                                                   \
  } TYPE_NAME##List;                                                           \
  errno_t TYPE_NAME##ListInit(TYPE_NAME##List *l, int initialSize) {           \
    if (initialSize < 0) {                                                     \
      return 1;                                                                \
    }                                                                          \
    l->len = 0;                                                                \
    l->cap = initialSize;                                                      \
    l->p = (T *)calloc(initialSize, sizeof(T));                                \
    if (l->p == NULL) {                                                        \
      return 1;                                                                \
    }                                                                          \
    return 0;                                                                  \
  }                                                                            \
  void TYPE_NAME##ListDestroy(TYPE_NAME##List *l) { free(l->p); }              \
  errno_t TYPE_NAME##ListAppend(TYPE_NAME##List *l, T item) {                  \
    if (l->len == l->cap) {                                                    \
      l->cap *= 2;                                                             \
      T *newP = (T *)calloc(l->cap, sizeof(T));                                \
      if (newP == NULL) {                                                      \
        return 1;                                                              \
      }                                                                        \
      for (int i = 0; i < l->len; ++i) {                                       \
        newP[i] = l->p[i];                                                     \
      }                                                                        \
      free(l->p);                                                              \
      l->p = newP;                                                             \
    }                                                                          \
    l->p[l->len] = item;                                                       \
    ++l->len;                                                                  \
    return 0;                                                                  \
  }                                                                            \
  TYPE_NAME##List TYPE_NAME##ListCopy(TYPE_NAME##List *src) {                  \
    TYPE_NAME##List dest;                                                      \
    dest.len = src->len;                                                       \
    dest.cap = src->len;                                                       \
    dest.p = (T *)calloc(dest.cap, sizeof(T));                                 \
    for (int i = 0; i < src->len; ++i) {                                       \
      dest.p[i] = src->p[i];                                                   \
    }                                                                          \
    return dest;                                                               \
  }                                                                            \
  errno_t TYPE_NAME##ListRemoveAt(TYPE_NAME##List *l, int index) {             \
    if (index < 0 || index >= l->len) {                                        \
      return 1;                                                                \
    }                                                                          \
    --l->len;                                                                  \
    for (int i = index; i < l->len; ++i) {                                     \
      l->p[i] = l->p[i + 1];                                                   \
    }                                                                          \
    return 0;                                                                  \
  }                                                                            \
  errno_t TYPE_NAME##ListInsertAt(TYPE_NAME##List *l, T item, int index) {     \
    if (index < 0 || index > l->len) {                                         \
      return 1;                                                                \
    }                                                                          \
    if (TYPE_NAME##ListAppend(l, l->p[l->len - 1])) {                          \
      return 1;                                                                \
    }                                                                          \
    for (int i = l->len - 2; i > index; --i) {                                 \
      l->p[i] = l->p[i - 1];                                                   \
    }                                                                          \
    l->p[index] = item;                                                        \
    return 0;                                                                  \
  }
