#ifndef FUNCTIONS5_H
#define FUNCTIONS5_H

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPEC_MAX_FLAGS 16
#define SPEC_MAX_DIGITS 6
#define BUF_START_CAP 64

typedef enum {
    STATUS_OK,
    STATUS_ERR_NULL,
    STATUS_ERR_FORMAT,
    STATUS_ERR_RANGE,
    STATUS_ERR_DIGIT,
    STATUS_ERR_OVERFLOW,
    STATUS_ERR_IO,
    STATUS_ERR_ALLOC
} status_t;

typedef enum {
    LEN_NONE,
    LEN_HH,
    LEN_H,
    LEN_L,
    LEN_LL
} length_t;

typedef struct {
    char* data;
    size_t len;
    size_t cap;
} strbuf_t;

int valid_base(int base){
    return base >= 2 && base <= 36;
}

int digit_val(char c, int upper){
    if (c >= '0' && c <= '9') return c - '0';
    if (upper && c >= 'A' && c <= 'Z') return c - 'A' + 10;
    if (!upper && c >= 'a' && c <= 'z') return c - 'a' + 10;
    return -1;
}

status_t roman_str(int n, char* out){
    static const int vals[13] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
    static const char* const syms[13] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};

    if (out == NULL) return STATUS_ERR_NULL;
    if (n < 1 || n > 3999) return STATUS_ERR_RANGE;

    size_t p = 0;
    for (int i = 0; i < 13; i++){
        while (n >= vals[i]){
            for (const char* s = syms[i]; *s != '\0'; s++) out[p++] = *s;
            n -= vals[i];
        }
    }
    out[p] = '\0';
    return STATUS_OK;
}

status_t zeckendorf_str(unsigned int n, char* out){
    if (out == NULL) return STATUS_ERR_NULL;
    if (n == 0) return STATUS_ERR_RANGE;

    unsigned long long fib[48];
    unsigned long long a = 1, b = 2;
    int k = 0;
    while (a <= n){
        fib[k++] = a;
        unsigned long long t = a + b;
        a = b;
        b = t;
    }

    unsigned long long rest = n;
    for (int i = k - 1; i >= 0; i--){
        if (rest >= fib[i]){
            out[i] = '1';
            rest -= fib[i];
        } else {
            out[i] = '0';
        }
    }
    out[k] = '1';
    out[k + 1] = '\0';
    return STATUS_OK;
}

status_t int_to_base(int n, int base, int upper, char* out){
    if (out == NULL) return STATUS_ERR_NULL;
    if (!valid_base(base)) base = 10;

    unsigned long long v = (n < 0) ? 0ULL - (unsigned long long)n : (unsigned long long)n;
    char tmp[40];
    int k = 0;
    do {
        int d = (int)(v % (unsigned long long)base);
        tmp[k++] = (char)(d < 10 ? '0' + d : (upper ? 'A' : 'a') + d - 10);
        v /= (unsigned long long)base;
    } while (v > 0);

    int p = 0;
    if (n < 0) out[p++] = '-';
    while (k > 0) out[p++] = tmp[--k];
    out[p] = '\0';
    return STATUS_OK;
}

status_t base_to_int(const char* s, int base, int upper, int* res){
    if (s == NULL || res == NULL) return STATUS_ERR_NULL;
    if (!valid_base(base)) base = 10;

    int neg = (s[0] == '-');
    if (s[neg] == '\0') return STATUS_ERR_DIGIT;

    unsigned long long limit = neg ? (unsigned long long)INT_MAX + 1 : (unsigned long long)INT_MAX;
    unsigned long long v = 0;

    for (const char* c = s + neg; *c != '\0'; c++){
        int d = digit_val(*c, upper);
        if (d < 0 || d >= base) return STATUS_ERR_DIGIT;
        if (v > (limit - (unsigned long long)d) / (unsigned long long)base) return STATUS_ERR_OVERFLOW;
        v = v * (unsigned long long)base + (unsigned long long)d;
    }

    if (neg && v == (unsigned long long)INT_MAX + 1) *res = INT_MIN;
    else *res = neg ? -(int)v : (int)v;
    return STATUS_OK;
}

status_t mem_dump(const void* p, size_t size, char* out){
    if (p == NULL || out == NULL) return STATUS_ERR_NULL;

    const unsigned char* bytes = (const unsigned char*)p;
    size_t k = 0;
    for (size_t i = 0; i < size; i++){
        if (i > 0) out[k++] = ' ';
        for (int bit = 7; bit >= 0; bit--) out[k++] = ((bytes[i] >> bit) & 1) ? '1' : '0';
    }
    out[k] = '\0';
    return STATUS_OK;
}

status_t buf_reserve(strbuf_t* b, size_t extra){
    if (b == NULL) return STATUS_ERR_NULL;
    if (extra > (size_t)INT_MAX || b->len > (size_t)INT_MAX - extra) return STATUS_ERR_OVERFLOW;

    size_t need = b->len + extra + 1;
    if (need <= b->cap) return STATUS_OK;

    size_t cap = (b->cap == 0) ? BUF_START_CAP : b->cap;
    while (cap < need) cap *= 2;

    char* tmp = (char*)realloc(b->data, cap);
    if (tmp == NULL) return STATUS_ERR_ALLOC;
    b->data = tmp;
    b->cap = cap;
    return STATUS_OK;
}

status_t buf_push(strbuf_t* b, const char* s, size_t n){
    if (b == NULL || (s == NULL && n > 0)) return STATUS_ERR_NULL;
    status_t st = buf_reserve(b, n);
    if (st != STATUS_OK) return st;
    if (n > 0) memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
    return STATUS_OK;
}

status_t buf_printf(strbuf_t* b, const char* spec, ...){
    va_list ap, cp;
    va_start(ap, spec);
    va_copy(cp, ap);
    int n = vsnprintf(NULL, 0, spec, cp);
    va_end(cp);

    status_t st = STATUS_OK;
    if (n < 0) st = STATUS_ERR_FORMAT;
    if (st == STATUS_OK) st = buf_reserve(b, (size_t)n);
    if (st == STATUS_OK){
        if (vsnprintf(b->data + b->len, (size_t)n + 1, spec, ap) != n) st = STATUS_ERR_FORMAT;
        else b->len += (size_t)n;
    }
    va_end(ap);
    return st;
}

status_t parse_spec(const char* p, char* spec, int* k_out, length_t* len, char* conv, size_t* used){
    size_t i = 0;
    int k = 0, digits = 0;
    spec[k++] = '%';

    while (p[i] != '\0' && strchr("-+ #0", p[i]) != NULL){
        if (k > SPEC_MAX_FLAGS) return STATUS_ERR_FORMAT;
        spec[k++] = p[i++];
    }
    while (p[i] >= '0' && p[i] <= '9'){
        if (++digits > SPEC_MAX_DIGITS) return STATUS_ERR_FORMAT;
        spec[k++] = p[i++];
    }
    if (p[i] == '.'){
        spec[k++] = p[i++];
        digits = 0;
        while (p[i] >= '0' && p[i] <= '9'){
            if (++digits > SPEC_MAX_DIGITS) return STATUS_ERR_FORMAT;
            spec[k++] = p[i++];
        }
    }

    *len = LEN_NONE;
    if (p[i] == 'h'){
        *len = (p[i + 1] == 'h') ? LEN_HH : LEN_H;
        i += (*len == LEN_HH) ? 2 : 1;
    } else if (p[i] == 'l'){
        *len = (p[i + 1] == 'l') ? LEN_LL : LEN_L;
        i += (*len == LEN_LL) ? 2 : 1;
    }

    if (p[i] == '\0') return STATUS_ERR_FORMAT;
    *conv = p[i++];
    *used = i;
    *k_out = k;
    return STATUS_OK;
}

status_t standard_spec(strbuf_t* b, const char* p, va_list* ap, size_t* used){
    char spec[48], conv = '\0';
    int k = 0;
    length_t len = LEN_NONE;

    status_t st = parse_spec(p, spec, &k, &len, &conv, used);
    if (st != STATUS_OK) return st;

    int is_sint = (strchr("di", conv) != NULL);
    int is_uint = (strchr("uoxX", conv) != NULL);
    int is_real = (strchr("fFeEgGaA", conv) != NULL);

    if (!is_sint && !is_uint && !is_real && strchr("csp", conv) == NULL && conv != 'n') return STATUS_ERR_FORMAT;
    if (is_real && len != LEN_NONE && len != LEN_L) return STATUS_ERR_FORMAT;
    if (!is_sint && !is_uint && !is_real && len != LEN_NONE) return STATUS_ERR_FORMAT;

    if (is_sint || is_uint){
        spec[k++] = 'l';
        spec[k++] = 'l';
    }
    spec[k++] = conv;
    spec[k] = '\0';

    if (is_sint){
        long long v;
        switch (len){
            case LEN_HH: v = (signed char)va_arg(*ap, int); break;
            case LEN_H:  v = (short)va_arg(*ap, int); break;
            case LEN_L:  v = va_arg(*ap, long); break;
            case LEN_LL: v = va_arg(*ap, long long); break;
            default:     v = va_arg(*ap, int); break;
        }
        return buf_printf(b, spec, v);
    }
    if (is_uint){
        unsigned long long v;
        switch (len){
            case LEN_HH: v = (unsigned char)va_arg(*ap, unsigned int); break;
            case LEN_H:  v = (unsigned short)va_arg(*ap, unsigned int); break;
            case LEN_L:  v = va_arg(*ap, unsigned long); break;
            case LEN_LL: v = va_arg(*ap, unsigned long long); break;
            default:     v = va_arg(*ap, unsigned int); break;
        }
        return buf_printf(b, spec, v);
    }
    if (is_real) return buf_printf(b, spec, va_arg(*ap, double));

    if (conv == 'c') return buf_printf(b, spec, va_arg(*ap, int));
    if (conv == 'p') return buf_printf(b, spec, va_arg(*ap, void*));
    if (conv == 'n'){
        if (k != 2) return STATUS_ERR_FORMAT;
        int* dst = va_arg(*ap, int*);
        if (dst == NULL) return STATUS_ERR_NULL;
        if (b->len > (size_t)INT_MAX) return STATUS_ERR_OVERFLOW;
        *dst = (int)b->len;
        return STATUS_OK;
    }

    const char* s = va_arg(*ap, const char*);
    if (s == NULL) s = "(null)";
    if (k == 2) return buf_push(b, s, strlen(s));
    return buf_printf(b, spec, s);
}

status_t custom_spec(strbuf_t* b, const char* p, va_list* ap, int* matched){
    char tmp[80];
    status_t st = STATUS_OK;
    *matched = 1;

    if (p[0] == 'R' && p[1] == 'o'){
        st = roman_str(va_arg(*ap, int), tmp);
    } else if (p[0] == 'Z' && p[1] == 'r'){
        st = zeckendorf_str(va_arg(*ap, unsigned int), tmp);
    } else if (p[0] == 'C' && (p[1] == 'v' || p[1] == 'V')){
        int n = va_arg(*ap, int);
        int base = va_arg(*ap, int);
        st = int_to_base(n, base, p[1] == 'V', tmp);
    } else if ((p[0] == 't' && p[1] == 'o') || (p[0] == 'T' && p[1] == 'O')){
        const char* s = va_arg(*ap, const char*);
        int base = va_arg(*ap, int);
        int v = 0;
        st = base_to_int(s, base, p[0] == 'T', &v);
        if (st == STATUS_OK) st = int_to_base(v, 10, 0, tmp);
    } else if (p[0] == 'm' && p[1] == 'i'){
        int v = va_arg(*ap, int);
        st = mem_dump(&v, sizeof v, tmp);
    } else if (p[0] == 'm' && p[1] == 'u'){
        unsigned int v = va_arg(*ap, unsigned int);
        st = mem_dump(&v, sizeof v, tmp);
    } else if (p[0] == 'm' && p[1] == 'd'){
        double v = va_arg(*ap, double);
        st = mem_dump(&v, sizeof v, tmp);
    } else if (p[0] == 'm' && p[1] == 'f'){
        float v = (float)va_arg(*ap, double);
        st = mem_dump(&v, sizeof v, tmp);
    } else {
        *matched = 0;
        return STATUS_OK;
    }

    if (st != STATUS_OK) return st;
    return buf_push(b, tmp, strlen(tmp));
}

status_t build_output(strbuf_t* b, const char* fmt, va_list* ap){
    status_t st = buf_push(b, "", 0);
    size_t i = 0;

    while (st == STATUS_OK && fmt[i] != '\0'){
        if (fmt[i] != '%'){
            st = buf_push(b, &fmt[i++], 1);
        } else if (fmt[i + 1] == '%'){
            st = buf_push(b, "%", 1);
            i += 2;
        } else {
            int matched = 0;
            size_t used = 0;
            st = custom_spec(b, &fmt[i + 1], ap, &matched);
            if (st == STATUS_OK && matched){
                used = 2;
            } else if (st == STATUS_OK){
                st = standard_spec(b, &fmt[i + 1], ap, &used);
            }
            i += 1 + used;
        }
    }
    return st;
}

int overfprintf(FILE* f, const char* fmt, ...){
    if (f == NULL || fmt == NULL) return -(int)STATUS_ERR_NULL;

    strbuf_t b = {NULL, 0, 0};
    va_list ap;
    va_start(ap, fmt);
    status_t st = build_output(&b, fmt, &ap);
    va_end(ap);

    if (st == STATUS_OK && b.len > 0 && fwrite(b.data, 1, b.len, f) != b.len) st = STATUS_ERR_IO;

    int res = (st == STATUS_OK) ? (int)b.len : -(int)st;
    free(b.data);
    return res;
}

int oversprintf(char* buf, const char* fmt, ...){
    if (buf == NULL || fmt == NULL) return -(int)STATUS_ERR_NULL;

    strbuf_t b = {NULL, 0, 0};
    va_list ap;
    va_start(ap, fmt);
    status_t st = build_output(&b, fmt, &ap);
    va_end(ap);

    if (st == STATUS_OK) memcpy(buf, b.data, b.len + 1);

    int res = (st == STATUS_OK) ? (int)b.len : -(int)st;
    free(b.data);
    return res;
}

void print_status(int res){
    if (res >= 0) return;
    switch ((status_t)(-res)){
        case STATUS_OK:           break;
        case STATUS_ERR_NULL:     fprintf(stderr, "Error: NULL pointer\n"); break;
        case STATUS_ERR_FORMAT:   fprintf(stderr, "Error: unknown or unsupported specifier\n"); break;
        case STATUS_ERR_RANGE:    fprintf(stderr, "Error: value out of range\n"); break;
        case STATUS_ERR_DIGIT:    fprintf(stderr, "Error: invalid digit for this base\n"); break;
        case STATUS_ERR_OVERFLOW: fprintf(stderr, "Error: result or number is too long\n"); break;
        case STATUS_ERR_IO:       fprintf(stderr, "Error: failed to write to file\n"); break;
        case STATUS_ERR_ALLOC:    fprintf(stderr, "Error: failed to allocate memory\n"); break;
        default:                  fprintf(stderr, "Error: unknown error\n"); break;
    }
}

#endif
