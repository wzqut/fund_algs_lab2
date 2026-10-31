#ifndef FUNCTIONS9_H
#define FUNCTIONS9_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_Q 1000000LL
#define MAX_EPS 5e-13

typedef enum {
    STATUS_OK,
    STATUS_ERR_NULL,
    STATUS_ERR_EPS,
    STATUS_ERR_BASE,
    STATUS_ERR_CNT,
    STATUS_ERR_RANGE,
    STATUS_ERR_FRAC,
    STATUS_ERR_ALLOC
} status_t;

double abs_d(double x){
    return (x < 0) ? -x : x;
}

status_t to_frac(double x, double eps, long long* p, long long* q){
    if (p == NULL || q == NULL) return STATUS_ERR_NULL;
    if (!(eps > 0 && eps < MAX_EPS)) return STATUS_ERR_EPS;
    if (!(x > 0 && x < 1)) return STATUS_ERR_RANGE;

    for (long long d = 2; d <= MAX_Q; d++){
        long long n = (long long)(x * (double)d + 0.5);
        if (n > 0 && n < d && abs_d(x - (double)n / (double)d) < eps){
            *p = n;
            *q = d;
            return STATUS_OK;
        }
    }
    return STATUS_ERR_FRAC;
}

int is_finite(long long q, int base){
    for (long long d = 2; d * d <= q; d++){
        if (q % d == 0){
            if (base % d != 0) return 0;
            while (q % d == 0) q /= d;
        }
    }
    if (q > 1 && base % q != 0) return 0;
    return 1;
}

status_t check_fin(int** res, double eps, int base, int cnt, ...){
    if (res == NULL) return STATUS_ERR_NULL;
    if (!(eps > 0 && eps < MAX_EPS)) return STATUS_ERR_EPS;
    if (base < 2) return STATUS_ERR_BASE;
    if (cnt < 1) return STATUS_ERR_CNT;

    int* r = (int*)malloc(sizeof(int) * (size_t)cnt);
    if (r == NULL) return STATUS_ERR_ALLOC;

    va_list ap;
    va_start(ap, cnt);
    for (int i = 0; i < cnt; i++){
        double x = va_arg(ap, double);
        long long p = 0, q = 1;
        status_t s = to_frac(x, eps, &p, &q);
        if (s != STATUS_OK){
            va_end(ap);
            free(r);
            return s;
        }
        r[i] = is_finite(q, base);
    }
    va_end(ap);

    *res = r;
    return STATUS_OK;
}

void print_res(status_t s, int base, int cnt, const double* nums, const int* res){
    switch (s){
        case STATUS_OK:
            if (nums == NULL || res == NULL){
                fprintf(stderr, "Error: NULL pointer\n");
                break;
            }
            printf("base %d:\n", base);
            for (int i = 0; i < cnt; i++)
                printf("  %.10g - %s\n", nums[i], res[i] ? "finite" : "infinite");
            break;
        case STATUS_ERR_NULL:  fprintf(stderr, "Error: NULL pointer\n"); break;
        case STATUS_ERR_EPS:   fprintf(stderr, "Error: eps must be in (0; %g)\n", MAX_EPS); break;
        case STATUS_ERR_BASE:  fprintf(stderr, "Error: base %d must be >= 2\n", base); break;
        case STATUS_ERR_CNT:   fprintf(stderr, "Error: count of numbers must be >= 1\n"); break;
        case STATUS_ERR_RANGE: fprintf(stderr, "Error: number must be in (0; 1)\n"); break;
        case STATUS_ERR_FRAC:  fprintf(stderr, "Error: cannot convert number to fraction p/q with q <= %lld\n", MAX_Q); break;
        case STATUS_ERR_ALLOC: fprintf(stderr, "Error: failed to allocate memory\n"); break;
        default:               fprintf(stderr, "Error: unknown error\n"); break;
    }
}

#endif
