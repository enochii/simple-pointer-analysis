#include <stdio.h>

struct test02 {
  int a;
};


int f() {
  struct test02 t;
  t.a = 0;
  return t.a;
}