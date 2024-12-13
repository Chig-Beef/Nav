#include <stdio.h>
#include <stdlib.h>

void panic(char *msg) {
  printf("%s\n", msg);
  exit(1);
}
