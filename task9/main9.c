#include "functions9.h"

int main(void){
    const double eps = 1e-13;
    int* r = NULL;
    status_t s;

    double n1[] = {0.125, 0.1, 0.3, 0.75};
    printf("1) base 10: 0.125, 0.1, 0.3, 0.75\n");
    s = check_fin(&r, eps, 10, 4, 0.125, 0.1, 0.3, 0.75);
    print_res(s, 10, 4, n1, r);
    free(r);
    r = NULL;

    double n2[] = {0.5, 0.125, 0.1};
    printf("\n2) base 2: 0.5, 0.125, 0.1\n");
    s = check_fin(&r, eps, 2, 3, 0.5, 0.125, 0.1);
    print_res(s, 2, 3, n2, r);
    free(r);
    r = NULL;

    double n3[] = {1.0 / 3, 2.0 / 9, 0.5};
    printf("\n3) base 3: 1/3, 2/9, 0.5\n");
    s = check_fin(&r, eps, 3, 3, 1.0 / 3, 2.0 / 9, 0.5);
    print_res(s, 3, 3, n3, r);
    free(r);
    r = NULL;

    double n4[] = {1.0 / 3, 0.25, 1.0 / 7, 5.0 / 12};
    printf("\n4) base 12: 1/3, 0.25, 1/7, 5/12\n");
    s = check_fin(&r, eps, 12, 4, 1.0 / 3, 0.25, 1.0 / 7, 5.0 / 12);
    print_res(s, 12, 4, n4, r);
    free(r);
    r = NULL;

    double n5[] = {0.123456, 0.999999};
    printf("\n5) base 10: 0.123456, 0.999999\n");
    s = check_fin(&r, eps, 10, 2, 0.123456, 0.999999);
    print_res(s, 10, 2, n5, r);
    free(r);
    r = NULL;

    printf("\n6) base 10: 0.5, 1.0\n");
    s = check_fin(&r, eps, 10, 2, 0.5, 1.0);
    print_res(s, 10, 2, NULL, r);
    free(r);
    r = NULL;

    printf("\n7) base 1: 0.5\n");
    s = check_fin(&r, eps, 1, 1, 0.5);
    print_res(s, 1, 1, NULL, r);
    free(r);
    r = NULL;

    printf("\n8) cnt = 0\n");
    s = check_fin(&r, eps, 10, 0);
    print_res(s, 10, 0, NULL, r);
    free(r);
    r = NULL;

    printf("\n9) eps = 0\n");
    s = check_fin(&r, 0.0, 10, 1, 0.5);
    print_res(s, 10, 1, NULL, r);
    free(r);
    r = NULL;

    printf("\n10) base 10: 0.1234567891\n");
    s = check_fin(&r, eps, 10, 1, 0.1234567891);
    print_res(s, 10, 1, NULL, r);
    free(r);
    r = NULL;

    return 0;
}
