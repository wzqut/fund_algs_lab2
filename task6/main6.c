#include <stdio.h>
#include "functions6.h"

int main(void){
    int r, a = 0, b = 0;
    unsigned int u = 0;

    printf("1) custom spec, valid input\n");
    r = oversscanf("XIV 1010011 ff", "%Ro %Zr %Cv", &a, &u, &b, 16);
    print_scan_result(r);
    printf("Values: %d %u %d\n", a, u, b);

    printf("\n2) mixed\n");
    double x = 0;
    r = oversscanf("year=XIV 3.5", "year=%Ro %lf", &a, &x);
    print_scan_result(r);
    printf("Values: %d %.1f\n", a, x);

    printf("\n3) 1/2 invalid data\n");
    print_scan_result(oversscanf("XIV abc", "%Ro %Ro", &a, &b));

    printf("\n4) base = 99 -> base = 10, %%CV\n");
    print_scan_result(oversscanf("1", "%Cv", &a, 99));
    r = oversscanf("-FF", "%CV", &a, 16);
    print_scan_result(r);
    printf("Values: %d\n", a);

    printf("\n5) invalid roman, no end marker, overflow\n");
    print_scan_result(oversscanf("IIII", "%Ro", &a));
    print_scan_result(oversscanf("1010", "%Zr", &u));
    print_scan_result(oversscanf("80000000", "%Cv", &a, 16));

    printf("\n6) errors: null, unknown spec\n");
    print_scan_result(oversscanf(NULL, "%d", &a));
    print_scan_result(oversscanf("1", "%q", &a));

    printf("\n7) empty\n");
    print_scan_result(oversscanf("", "%Ro", &a));

    printf("\n8) file\n");
    FILE* f = fopen("test6.txt", "w");
    if (f == NULL){
        fprintf(stderr, "Error: cannot create test6.txt\n");
        return 1;
    }
    fprintf(f, "7 XLII -1f\n");
    fclose(f);

    f = fopen("test6.txt", "r");
    if (f == NULL){
        fprintf(stderr, "Error: cannot open test6.txt\n");
        return 1;
    }
    int c = 0;
    r = overfscanf(f, "%d %Ro %Cv", &a, &b, &c, 16);
    fclose(f);
    print_scan_result(r);
    printf("Values: %d %d %d\n", a, b, c);

    return 0;
}
