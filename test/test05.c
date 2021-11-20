int f(int op) {
  int x, z;
  int *y;
  int** b = &y;
  if(1 < op)
    y = &x;
  else 
    y = &z;
  int** c = &y;
  return **c + **b;;
}