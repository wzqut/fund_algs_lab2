#include <stdio.h>
#include "functions5.h"

int main(void){
    char buf[512];
    int r;

    printf("1) %%Ro, %%Zr\n");
    r = oversprintf(buf, "%Ro %Ro %Ro | %Zr %Zr", 1994, 4, 3999, 17u, 1u);
    print_status(r);
    if (r >= 0) printf("%s (%d)\n", buf, r);

    printf("\n2) %%Cv, %%CV, %%to, %%TO\n");
    r = oversprintf(buf, "%Cv %CV %Cv | %to %TO", 255, 16, 255, 16, -255, 2, "ff", 16, "FF", 16);
    print_status(r);
    if (r >= 0) printf("%s (%d)\n", buf, r);

    printf("\n3) %%mi, %%mu, %%mf, %%md\n");
    r = oversprintf(buf, "%mi\n%mu\n%mf\n%md", 5, 4294967295u, 1.0f, 1.0);
    print_status(r);
    if (r >= 0) printf("%s (%d)\n", buf, r);

    printf("\n4) custom and standard\n");
    r = oversprintf(buf, "[%5d|%-5d|%05.1f|%s|%c|%x|%ld|%hhd] year %Ro, 100%%", 42, 42, 3.14159, "text", 'Z', 255, 123456789L, 300, 2024);
    print_status(r);
    if (r >= 0) printf("%s (%d)\n", buf, r);

    printf("\n5) overfprintf to stdout\n");
    r = overfprintf(stdout, "Hello %Ro, %Cv, %to\n", 14, 255, 16, "1a", 16);
    print_status(r);

    printf("\n6) errors\n");
    print_status(oversprintf(buf, "%Ro", 0));
    print_status(oversprintf(buf, "%Zr", 0u));
    print_status(oversprintf(buf, "%to", "FF", 16));
    print_status(oversprintf(buf, "%to", "99999999999", 10));
    print_status(oversprintf(buf, "%q", 1));
    print_status(oversprintf(buf, "100%"));
    print_status(oversprintf(NULL, "x"));
    print_status(overfprintf(stdout, NULL));

    return 0;
}
