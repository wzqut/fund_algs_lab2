#include "functions10.h"

#define EPS 1e-9

int main(void){
    double* g = NULL;
    status_t s;

    double f1[] = {2.0, 3.0, 1.0};
    printf("1) f(x) = 2x^2 + 3x + 1, a = 4\n");
    s = decompose(EPS, 4.0, &g, 2, 2.0, 3.0, 1.0);
    print_result(s, f1, g, 2, 4.0, 6.0, EPS);
    free(g);
    g = NULL;

    double f2[] = {4.0, 3.0, 2.0, 1.0};
    printf("\n2) f(x) = 4x^3 + 3x^2 + 2x + 1, a = -2\n");
    s = decompose(EPS, -2.0, &g, 3, 4.0, 3.0, 2.0, 1.0);
    print_result(s, f2, g, 3, -2.0, 1.5, EPS);
    free(g);
    g = NULL;

    double f3[] = {5.0};
    printf("\n3) f(x) = 5, a = 10\n");
    s = decompose(EPS, 10.0, &g, 0, 5.0);
    print_result(s, f3, g, 0, 10.0, 3.0, EPS);
    free(g);
    g = NULL;

    double f4[] = {1.0, 0.0, 0.0, 0.0, -1.0};
    printf("\n4) f(x) = x^4 - 1, a = 0.5\n");
    s = decompose(EPS, 0.5, &g, 4, 1.0, 0.0, 0.0, 0.0, -1.0);
    print_result(s, f4, g, 4, 0.5, 2.0, EPS);
    free(g);
    g = NULL;

    printf("\n5) degree = -1\n");
    s = decompose(EPS, 1.0, &g, -1);
    print_result(s, NULL, g, -1, 1.0, 0.0, EPS);
    free(g);
    g = NULL;

    printf("\n6) eps = 0\n");
    s = decompose(0.0, 1.0, &g, 1, 1.0, 1.0);
    print_result(s, NULL, g, 1, 1.0, 0.0, EPS);
    free(g);
    g = NULL;

    return 0;
}
