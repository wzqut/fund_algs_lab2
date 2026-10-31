#include "functions3.h"

int main(void){
    FileRes* r = NULL;
    status_t s;

    printf("1) \"ab\" in t3_in1.txt, t3_in2.txt\n");
    s = find_all(&r, "ab", 2, "t3_in1.txt", "t3_in2.txt");
    print_res(s, r, 2);
    if (s == STATUS_OK) free_res(r, 2);
    r = NULL;

    printf("\n2) \"b\\nx\" in t3_in1.txt\n");
    s = find_all(&r, "b\nx", 1, "t3_in1.txt");
    print_res(s, r, 1);
    if (s == STATUS_OK) free_res(r, 1);
    r = NULL;

    printf("\n3) \"aa\" in t3_in2.txt, t3_in3.txt (t3_in3.txt is empty)\n");
    s = find_all(&r, "aa", 2, "t3_in2.txt", "t3_in3.txt");
    print_res(s, r, 2);
    if (s == STATUS_OK) free_res(r, 2);
    r = NULL;

    printf("\n4) \" \\t \" in t3_in4.txt\n");
    s = find_all(&r, " \t ", 1, "t3_in4.txt");
    print_res(s, r, 1);
    if (s == STATUS_OK) free_res(r, 1);
    r = NULL;

    printf("\n5) \"\xd0\xb4\xd0\xb0\" (UTF-8) in t3_in4.txt\n");
    s = find_all(&r, "\xd0\xb4\xd0\xb0", 1, "t3_in4.txt");
    print_res(s, r, 1);
    if (s == STATUS_OK) free_res(r, 1);
    r = NULL;

    printf("\n6) \"\" in t3_in1.txt\n");
    s = find_all(&r, "", 1, "t3_in1.txt");
    print_res(s, r, 1);
    if (s == STATUS_OK) free_res(r, 1);
    r = NULL;

    printf("\n7) \"ab\" in t3_in1.txt, nofile.txt\n");
    s = find_all(&r, "ab", 2, "t3_in1.txt", "nofile.txt");
    print_res(s, r, 2);
    if (s == STATUS_OK) free_res(r, 2);
    r = NULL;

    printf("\n8) cnt = 0\n");
    s = find_all(&r, "ab", 0);
    print_res(s, r, 0);
    if (s == STATUS_OK) free_res(r, 0);
    r = NULL;

    printf("\n9) sub = NULL\n");
    s = find_all(&r, NULL, 1, "t3_in1.txt");
    print_res(s, r, 1);
    if (s == STATUS_OK) free_res(r, 1);

    return 0;
}
