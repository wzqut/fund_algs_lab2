#include "functions8.h"

int main(void){
    char* r = NULL;
    status_t s;

    printf("1) base 10: 999 + 1 + 0000\n");
    s = sum(&r, 10, 3, "999", "1", "0000");
    print_sum(s, 10, r);
    free(r);
    r = NULL;

    printf("\n2) base 16: ff + Ff + 2\n");
    s = sum(&r, 16, 3, "ff", "Ff", "2");
    print_sum(s, 16, r);
    free(r);
    r = NULL;

    printf("\n3) base 10: 99999999999999999999999999999999 + 1 + 00000000000000000000000000000001\n");
    s = sum(&r, 10, 3, "99999999999999999999999999999999", "1", "00000000000000000000000000000001");
    print_sum(s, 10, r);
    free(r);
    r = NULL;

    printf("\n4) base 2: 0 + 000\n");
    s = sum(&r, 2, 2, "0", "000");
    print_sum(s, 2, r);
    free(r);
    r = NULL;

    printf("\n5) base 36: zz + 1\n");
    s = sum(&r, 36, 2, "zz", "1");
    print_sum(s, 36, r);
    free(r);
    r = NULL;

    printf("\n6) base 10: 123 + 12A\n");
    s = sum(&r, 10, 2, "123", "12A");
    print_sum(s, 10, r);
    free(r);
    r = NULL;

    printf("\n7) base 37: 1 + 1\n");
    s = sum(&r, 37, 2, "1", "1");
    print_sum(s, 37, r);
    free(r);
    r = NULL;

    printf("\n8) cnt = 0\n");
    s = sum(&r, 10, 0);
    print_sum(s, 10, r);
    free(r);
    r = NULL;

    return 0;
}
