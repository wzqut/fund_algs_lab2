#ifndef FUNCTIONS1_H
#define FUNCTIONS1_H

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    STATUS_OK,
    STATUS_ERR_ARGC,
    STATUS_ERR_FLAG,
    STATUS_ERR_NULL,
    STATUS_ERR_NAN,
    STATUS_ERR_OVERFLOW,
    STATUS_ERR_NOT_ASCII,
    STATUS_ERR_ALLOC
} status_t;

int is_digit_s(char c){
    return c >= '0' && c <= '9';
}

int is_alpha_s(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

char to_upper_s(char c){
    if (c >= 'a' && c <= 'z')
        return (char)(c - 'a' + 'A');
    return c;
}

status_t check_ascii(const char* s){
    if (s == NULL) return STATUS_ERR_NULL;

    for (size_t i = 0; s[i] != '\0'; i++)
        if ((unsigned char)s[i] > 127) return STATUS_ERR_NOT_ASCII;
    return STATUS_OK;
}

status_t parse_uint(const char* s, unsigned int* res){
    if (s == NULL || res == NULL) return STATUS_ERR_NULL;
    if (s[0] == '\0') return STATUS_ERR_NAN;

    unsigned int v = 0;
    for (size_t i = 0; s[i] != '\0'; i++){
        if (!is_digit_s(s[i])) return STATUS_ERR_NAN;
        unsigned int d = (unsigned int)(s[i] - '0');
        if (v > (UINT_MAX - d) / 10) return STATUS_ERR_OVERFLOW;
        v = v * 10 + d;
    }

    *res = v;
    return STATUS_OK;
}

size_t raw_len(const char* s){
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

status_t alloc_copy_buf(const char* s, char** out, size_t* n, char** buf){
    if (s == NULL || out == NULL) return STATUS_ERR_NULL;
    status_t st = check_ascii(s);
    if (st != STATUS_OK) return st;

    *n = raw_len(s);
    if (*n == ((size_t)-1)) return STATUS_ERR_OVERFLOW;

    *buf = (char*)malloc(*n + 1);
    if (*buf == NULL) return STATUS_ERR_ALLOC;
    return STATUS_OK;
}

status_t str_len(const char* s, size_t* len){
    if (s == NULL || len == NULL) return STATUS_ERR_NULL;
    status_t st = check_ascii(s);
    if (st != STATUS_OK) return st;

    *len = raw_len(s);
    return STATUS_OK;
}

status_t str_reverse(const char* s, char** out){
    size_t n = 0;
    char* r = NULL;
    status_t st = alloc_copy_buf(s, out, &n, &r);
    if (st != STATUS_OK) return st;

    for (size_t i = 0; i < n; i++) r[i] = s[n - 1 - i];
    r[n] = '\0';

    *out = r;
    return STATUS_OK;
}

status_t str_odd_upper(const char* s, char** out){
    size_t n = 0;
    char* r = NULL;
    status_t st = alloc_copy_buf(s, out, &n, &r);
    if (st != STATUS_OK) return st;

    for (size_t i = 0; i < n; i++)
        r[i] = (i % 2 == 1) ? to_upper_s(s[i]) : s[i];
    r[n] = '\0';

    *out = r;
    return STATUS_OK;
}

status_t str_sort_types(const char* s, char** out){
    size_t n = 0;
    char* r = NULL;
    status_t st = alloc_copy_buf(s, out, &n, &r);
    if (st != STATUS_OK) return st;

    size_t k = 0;
    for (size_t i = 0; i < n; i++)
        if (is_digit_s(s[i])) r[k++] = s[i];
    for (size_t i = 0; i < n; i++)
        if (is_alpha_s(s[i])) r[k++] = s[i];
    for (size_t i = 0; i < n; i++)
        if (!is_digit_s(s[i]) && !is_alpha_s(s[i])) r[k++] = s[i];
    r[k] = '\0';

    *out = r;
    return STATUS_OK;
}

status_t str_concat_random(const char* const* strs, int cnt, unsigned int seed, char** out){
    if (strs == NULL || out == NULL || cnt < 1) return STATUS_ERR_NULL;

    const char** ord = (const char**)malloc(sizeof(const char*) * (size_t)cnt);
    if (ord == NULL) return STATUS_ERR_ALLOC;

    size_t total = 0;
    for (int i = 0; i < cnt; i++){
        if (strs[i] == NULL){
            free(ord);
            return STATUS_ERR_NULL;
        }
        status_t st = check_ascii(strs[i]);
        if (st != STATUS_OK){
            free(ord);
            return st;
        }
        ord[i] = strs[i];

        size_t n = raw_len(strs[i]);
        if (total > ((size_t)-1) - 1 - n){
            free(ord);
            return STATUS_ERR_OVERFLOW;
        }
        total += n;
    }

    srand(seed);
    for (int i = cnt - 1; i > 0; i--){
        int j = rand() % (i + 1);
        const char* t = ord[i];
        ord[i] = ord[j];
        ord[j] = t;
    }

    char* r = (char*)malloc(total + 1);
    if (r == NULL){
        free(ord);
        return STATUS_ERR_ALLOC;
    }

    size_t k = 0;
    for (int i = 0; i < cnt; i++)
        for (size_t j = 0; ord[i][j] != '\0'; j++)
            r[k++] = ord[i][j];
    r[k] = '\0';

    free(ord);
    *out = r;
    return STATUS_OK;
}

void print_error(status_t s){
    switch (s){
        case STATUS_ERR_ARGC:      fprintf(stderr, "Error: wrong number of arguments\n"); break;
        case STATUS_ERR_FLAG:      fprintf(stderr, "Error: unknown flag\n"); break;
        case STATUS_ERR_NULL:      fprintf(stderr, "Error: invalid parameters\n"); break;
        case STATUS_ERR_NAN:       fprintf(stderr, "Error: seed is not a valid unsigned int\n"); break;
        case STATUS_ERR_OVERFLOW:  fprintf(stderr, "Error: overflow\n"); break;
        case STATUS_ERR_NOT_ASCII: fprintf(stderr, "Error: only ASCII characters are allowed\n"); break;
        case STATUS_ERR_ALLOC:     fprintf(stderr, "Error: failed to allocate memory\n"); break;
        case STATUS_OK:            break;
        default:                   fprintf(stderr, "Error: unknown error\n"); break;
    }
}

void print_len(const char* str, size_t len){
    printf("Length of \"%s\": %zu\n", str, len);
}

void print_str(const char* name, const char* res){
    printf("%s: %s\n", name, res);
}

#endif
