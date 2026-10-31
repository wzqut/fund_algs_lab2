#ifndef FUNCTIONS8_H
#define FUNCTIONS8_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    STATUS_OK,
    STATUS_ERR_NULL,
    STATUS_ERR_BASE,
    STATUS_ERR_CNT,
    STATUS_ERR_DIGIT,
    STATUS_ERR_ALLOC,
    STATUS_ERR_OVERFLOW
} status_t;

size_t len_s(const char* s){
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

int digit(char c){
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return -1;
}

char to_char(int d){
    if (d < 10) return (char)('0' + d);
    return (char)('A' + d - 10);
}

status_t check_num(const char* s, int base){
    if (s == NULL) return STATUS_ERR_NULL;
    if (s[0] == '\0') return STATUS_ERR_DIGIT;
    for (size_t i = 0; s[i] != '\0'; i++){
        int d = digit(s[i]);
        if (d < 0 || d >= base) return STATUS_ERR_DIGIT;
    }
    return STATUS_OK;
}

void trim_zeros(char* s){
    size_t k = 0;
    while (s[k] == '0' && s[k + 1] != '\0') k++;
    if (k == 0) return;
    size_t i = 0;
    while (s[i + k] != '\0'){
        s[i] = s[i + k];
        i++;
    }
    s[i] = '\0';
}

status_t add_col(const char* a, const char* b, int base, char** res){
    if (a == NULL || b == NULL || res == NULL) return STATUS_ERR_NULL;
    if (base < 2 || base > 36) return STATUS_ERR_BASE;
    if (check_num(a, base) != STATUS_OK || check_num(b, base) != STATUS_OK) return STATUS_ERR_DIGIT;

    size_t la = len_s(a);
    size_t lb = len_s(b);
    size_t mx = (la > lb) ? la : lb;
    if (mx > SIZE_MAX - 2) return STATUS_ERR_OVERFLOW;

    char* r = (char*)malloc(mx + 2);
    if (r == NULL) return STATUS_ERR_ALLOC;

    size_t i = la, j = lb, k = 0;
    int carry = 0;
    while (i > 0 || j > 0 || carry){
        int da = (i > 0) ? digit(a[--i]) : 0;
        int db = (j > 0) ? digit(b[--j]) : 0;
        int s = da + db + carry;
        r[k++] = to_char(s % base);
        carry = s / base;
    }
    if (k == 0) r[k++] = '0';
    r[k] = '\0';

    for (size_t l = 0, h = k - 1; l < h; l++, h--){
        char t = r[l];
        r[l] = r[h];
        r[h] = t;
    }

    trim_zeros(r);
    *res = r;
    return STATUS_OK;
}

status_t sum(char** res, int base, int cnt, ...){
    if (res == NULL) return STATUS_ERR_NULL;
    if (base < 2 || base > 36) return STATUS_ERR_BASE;
    if (cnt < 1) return STATUS_ERR_CNT;

    char* acc = (char*)malloc(2);
    if (acc == NULL) return STATUS_ERR_ALLOC;
    acc[0] = '0';
    acc[1] = '\0';

    va_list ap;
    va_start(ap, cnt);
    for (int i = 0; i < cnt; i++){
        const char* x = va_arg(ap, const char*);
        char* tmp = NULL;
        status_t s = add_col(acc, x, base, &tmp);
        if (s != STATUS_OK){
            va_end(ap);
            free(acc);
            return s;
        }
        free(acc);
        acc = tmp;
    }
    va_end(ap);

    *res = acc;
    return STATUS_OK;
}

void print_sum(status_t s, int base, const char* res){
    switch (s){
        case STATUS_OK:           printf("Result (base %d): %s\n", base, res); break;
        case STATUS_ERR_NULL:     fprintf(stderr, "Error: NULL pointer\n"); break;
        case STATUS_ERR_BASE:     fprintf(stderr, "Error: base %d is not in [2..36]\n", base); break;
        case STATUS_ERR_CNT:      fprintf(stderr, "Error: count of numbers must be >= 1\n"); break;
        case STATUS_ERR_DIGIT:    fprintf(stderr, "Error: invalid number for base %d\n", base); break;
        case STATUS_ERR_ALLOC:    fprintf(stderr, "Error: failed to allocate memory\n"); break;
        case STATUS_ERR_OVERFLOW: fprintf(stderr, "Error: number is too long\n"); break;
        default:                  fprintf(stderr, "Error: unknown error\n"); break;
    }
}

#endif
