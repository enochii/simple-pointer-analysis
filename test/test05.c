int f(int op) {
  int x, z;
  int *y, *w;
  int** b = &y;
  if(1 < op) {
    y = &x;
    b = &w;
  }
  else 
    y = &z;
  return **b;;
}