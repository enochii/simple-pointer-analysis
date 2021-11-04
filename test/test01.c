#include <stdio.h>

struct A {
  int x;
  int y;
};

int f(int u) {
  struct A a;
  a.x = 1;
  a.y = 2;
  int b, *p;
  p = &b;
  *p = 1;
  int c = 1;
  if(1 < u) {
    c = 2;
  }
  return a.x + c;
}