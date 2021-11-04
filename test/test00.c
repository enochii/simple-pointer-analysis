#include <stdio.h>

int** f(int op, int a, int b) {
  int *p=&a, *q=&b;
  int **w;
  if(op < 2) {
    w = &p;
  } else {
    w = &q;
  }
  *w = &a;
  return w;
}