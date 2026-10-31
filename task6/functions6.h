#ifndef FUNCTIONS6_H
#define FUNCTIONS6_H

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define ZECK_MAX_DIGITS 47
#define ROMAN_MAX_LEN 15

typedef enum {
    STATUS_OK,
    STATUS_EOF,
    STATUS_ERR_NULL,
    STATUS_ERR_FORMAT,
    STATUS_ERR_MATCH
} status_t;

typedef struct {
    FILE* f;
    const char* s;
} reader_t;

int peek(const reader_t* r){
    if (r->f != NULL){
        int c = fgetc(r->f);
        if (c != EOF) ungetc(c, r->f);
        return c;
    }
    return (*r->s == '\0') ? EOF : (unsigned char)*r->s;
}

void take(reader_t* r){
    if (r->f != NULL) fgetc(r->f);
    else r->s++;
}

int is_space(int c){
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

void skip_spaces(reader_t* r){
    while (is_space(peek(r))) take(r);
}

int digit_val(int c, int upper){
    if (c >= '0' && c <= '9') return c - '0';
    if (upper && c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (!upper && c >= 'a' && c <= 'z') return c - 'a' + 10;
    return -1;
}

status_t nothing_read(const reader_t* r){
    return (peek(r) == EOF) ? STATUS_EOF : STATUS_ERR_MATCH;
}

int roman_val(int c){
    switch (c){
        case 'I': return 1;
        case 'V': return 5;
        case 'X': return 10;
        case 'L': return 50;
        case 'C': return 100;
        case 'D': return 500;
        case 'M': return 1000;
        default: return 0;
    }
}

void roman_str(int n, char* out){
    static const int vals[13] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
    static const char* const syms[13] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
    size_t p = 0;
    for (int i = 0; i < 13; i++){
        while (n >= vals[i]){
            for (const char* s = syms[i]; *s != '\0'; s++) out[p++] = *s;
            n -= vals[i];
        }
    }
    out[p] = '\0';
}

status_t scan_roman(reader_t* r, int* out){
    if (out == NULL) return STATUS_ERR_NULL;
    skip_spaces(r);

    char buf[ROMAN_MAX_LEN + 1];
    size_t len = 0;
    int c;
    while (roman_val(c = peek(r)) != 0){
        if (len >= ROMAN_MAX_LEN) return STATUS_ERR_MATCH;
        buf[len++] = (char)c;
        take(r);
    }
    if (len == 0) return nothing_read(r);
    buf[len] = '\0';

    int total = 0;
    for (size_t i = 0; i < len; i++){
        int cur = roman_val(buf[i]);
        int next = roman_val(buf[i + 1]);
        total += (cur < next) ? -cur : cur;
    }

    char canon[ROMAN_MAX_LEN + 1];
    if (total < 1 || total > 3999) return STATUS_ERR_MATCH;
    roman_str(total, canon);
    if (strcmp(canon, buf) != 0) return STATUS_ERR_MATCH;

    *out = total;
    return STATUS_OK;
}

status_t scan_zeckendorf(reader_t* r, unsigned int* out){
    if (out == NULL) return STATUS_ERR_NULL;
    skip_spaces(r);

    unsigned long long a = 1, b = 2, sum = 0;
    int c, prev = 0, digits = 0;
    while ((c = peek(r)) == '0' || c == '1'){
        take(r);
        digits++;
        if (prev == '1' && c == '1'){
            *out = (unsigned int)sum;
            return STATUS_OK;
        }
        if (c == '1') sum += a;
        if (sum > UINT_MAX || digits > ZECK_MAX_DIGITS) return STATUS_ERR_MATCH;

        unsigned long long t = a + b;
        a = b;
        b = t;
        prev = c;
    }
    return (digits == 0) ? nothing_read(r) : STATUS_ERR_MATCH;
}

status_t scan_base(reader_t* r, int* out, int base, int upper){
    if (out == NULL) return STATUS_ERR_NULL;
    if (base < 2 || base > 36) base = 10;
    skip_spaces(r);

    int neg = (peek(r) == '-');
    if (neg || peek(r) == '+') take(r);

    unsigned long long limit = neg ? (unsigned long long)INT_MAX + 1 : (unsigned long long)INT_MAX;
    unsigned long long v = 0;
    int c, d, digits = 0;

    while ((c = peek(r)) != EOF && (d = digit_val(c, upper)) >= 0 && d < base){
        if (v > (limit - (unsigned long long)d) / (unsigned long long)base) return STATUS_ERR_MATCH;
        v = v * (unsigned long long)base + (unsigned long long)d;
        digits++;
        take(r);
    }
    if (digits == 0) return nothing_read(r);

    *out = neg ? (int)(-(long long)v) : (int)v;
    return STATUS_OK;
}

status_t custom_scan(reader_t* r, const char* p, va_list* ap, int* matched, int* count){
    status_t st;
    *matched = 1;
    if (p[0] == 'R' && p[1] == 'o'){
        st = scan_roman(r, va_arg(*ap, int*));
    } else if (p[0] == 'Z' && p[1] == 'r'){
        st = scan_zeckendorf(r, va_arg(*ap, unsigned int*));
    } else if (p[0] == 'C' && (p[1] == 'v' || p[1] == 'V')){
        int* out = va_arg(*ap, int*);
        int base = va_arg(*ap, int);
        st = scan_base(r, out, base, p[1] == 'V');
    } else {
        *matched = 0;
        return STATUS_OK;
    }

    if (st == STATUS_OK) (*count)++;
    return st;
}

status_t standard_scan(reader_t* r, const char* p, va_list* ap, size_t* used, int* count){
    char spec[32];
    size_t i = 0;
    spec[0] = '%';

    while (p[i] != '\0' && strchr("*0123456789hlL", p[i]) != NULL){
        if (i >= 26) return STATUS_ERR_FORMAT;
        spec[i + 1] = p[i];
        i++;
    }
    if (p[i] == '\0' || strchr("diouxXeEfFgGcsp", p[i]) == NULL) return STATUS_ERR_FORMAT;
    spec[i + 1] = p[i];
    strcpy(spec + i + 2, "%n");
    *used = i + 1;

    int suppress = (strchr(spec, '*') != NULL);
    void* target = suppress ? NULL : va_arg(*ap, void*);
    if (!suppress && target == NULL) return STATUS_ERR_NULL;

    int n = -1, res;
    if (r->f != NULL) res = suppress ? fscanf(r->f, spec, &n) : fscanf(r->f, spec, target, &n);
    else res = suppress ? sscanf(r->s, spec, &n) : sscanf(r->s, spec, target, &n);

    if (n < 0) return (res == EOF) ? STATUS_EOF : STATUS_ERR_MATCH;
    if (r->f == NULL) r->s += n;
    if (!suppress) (*count)++;
    return STATUS_OK;
}

status_t scan_input(reader_t* r, const char* fmt, va_list* ap, int* count){
    *count = 0;
    size_t i = 0;

    while (fmt[i] != '\0'){
        if (is_space(fmt[i])){
            skip_spaces(r);
            i++;
        } else if (fmt[i] != '%' || fmt[i + 1] == '%'){
            if (fmt[i] == '%') skip_spaces(r);
            int c = peek(r);
            if (c == EOF) return STATUS_EOF;
            if (c != (unsigned char)fmt[i]) return STATUS_ERR_MATCH;
            take(r);
            i += (fmt[i] == '%') ? 2 : 1;
        } else {
            int matched = 0;
            size_t used = 0;
            status_t st = custom_scan(r, &fmt[i + 1], ap, &matched, count);
            if (st == STATUS_OK && matched) used = 2;
            else if (st == STATUS_OK) st = standard_scan(r, &fmt[i + 1], ap, &used, count);
            if (st != STATUS_OK) return st;

            i += 1 + used;
        }
    }
    return STATUS_OK;
}

int finish_scan(status_t st, int count){
    if (st == STATUS_EOF && count == 0) return EOF;
    if (st == STATUS_ERR_NULL || st == STATUS_ERR_FORMAT) return -(int)st;
    return count;
}

int overfscanf(FILE* f, const char* fmt, ...){
    if (f == NULL || fmt == NULL) return -(int)STATUS_ERR_NULL;

    reader_t r = {f, NULL};
    int count;
    va_list ap;
    va_start(ap, fmt);
    status_t st = scan_input(&r, fmt, &ap, &count);
    va_end(ap);
    return finish_scan(st, count);
}

int oversscanf(const char* str, const char* fmt, ...){
    if (str == NULL || fmt == NULL) return -(int)STATUS_ERR_NULL;

    reader_t r = {NULL, str};
    int count;
    va_list ap;
    va_start(ap, fmt);
    status_t st = scan_input(&r, fmt, &ap, &count);
    va_end(ap);
    return finish_scan(st, count);
}

void print_scan_result(int res){
    fflush(stdout);
    if (res >= 0) printf("Stored values: %d\n", res);
    else if (res == EOF) fprintf(stderr, "Error: input ended before the first value\n");
    else if (-res == STATUS_ERR_NULL) fprintf(stderr, "Error: NULL pointer\n");
    else fprintf(stderr, "Error: unknown or unsupported specifier\n");
}

#endif
