#include <stdio.h>

struct A {
  int x;
  int y;
};

int f(int u, int *q, int *p) {
  struct A a;
  a.x = 1;
  a.y = 2;
  int b;
  p = &b;
  *p = 1;
  q = p;
  int c = 1;
  if(1 < u) {
    c = 2;
    q = &c;
  }
  p = q;
  if(1 >= u) {
    p = &a.x;
  }
  return a.x + *p;
}