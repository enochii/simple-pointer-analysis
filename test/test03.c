typedef struct {
  int a;
  int* p;
} test03;

int* f(int op) {
  test03 s;
  int a, b;
  if(op < 1) {
    s.p = &a;
  } else {
    s.p = &b;
  }
  return s.p;
}

// struct with no pointer?