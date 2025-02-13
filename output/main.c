#include <string.h>

void main() {
  int x = 7;
  if (x==7) {
  } else if (x==3) {
  } else {
  }

  for ( int i = 0; i<7; ++i) {
    x = 7;
    if (x==7) {
    } else if (x==3) {
    } else {
      continue;
    }

  }

  for ( ; ; ) {
    x = 7;
    if (x==7) {
    } else if (x==3) {
    } else {
    }

    break;
  }

  for ( ; x<10; ) {
    ++x;
  }

}

