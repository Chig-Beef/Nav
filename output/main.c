#include <string.h>
#include <stdbool.h>

void main() {
  int x = 7;
  if (true) {
    x = 7;
  }

  if (x==7) {
    x = 7;
  }

  if (x!=7) {
    x = 7;
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
    break;
  }

  for ( ; x<10; ) {
    ++x;
  }

}

