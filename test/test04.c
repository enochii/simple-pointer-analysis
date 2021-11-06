typedef struct {
  int a;
  int* p;
} test03;

/// TODO: handle return a struct containing a pointer 
test03 f(int op) {
  test03 s;
  int a, b;
  if(op < 1) {
    s.p = &a;
  } else {
    s.p = &b;
  }
  return s;
}

// struct with no pointer?