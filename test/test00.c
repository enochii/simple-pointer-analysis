#include <stdio.h>

int main() {
    int a = 1;
    int *p = &a;
    *p = 1211;
    printf("%p\n", p);
    // auto q = p;
    // q = &b;
    
    return 0;
}