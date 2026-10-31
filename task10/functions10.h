#ifndef FUNCTIONS10_H
#define FUNCTIONS10_H

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    STATUS_OK,
    STATUS_ERR_NULL,
    STATUS_ERR_DEGREE,
    STATUS_ERR_EPS,
    STATUS_ERR_ALLOC
} status_t;

double abs_d(double x){
    return (x < 0) ? -x : x;
}

status_t poly_value(const double* c, int n, double x, double* res){
    if (c == NULL || res == NULL) return STATUS_ERR_NULL;
    if (n < 0) return STATUS_ERR_DEGREE;

    double v = 0;
    for (int i = 0; i <= n; i++)
        v = v * x + c[i];

    *res = v;
    return STATUS_OK;
}

status_t decompose(double eps, double a, double** res, int n, ...){
    if (res == NULL) return STATUS_ERR_NULL;
    if (!(eps > 0)) return STATUS_ERR_EPS;
    if (n < 0 || n == INT_MAX) return STATUS_ERR_DEGREE;

    double* c = (double*)malloc(sizeof(double) * ((size_t)n + 1));
    if (c == NULL) return STATUS_ERR_ALLOC;

    va_list ap;
    va_start(ap, n);
    for (int i = 0; i <= n; i++)
        c[i] = va_arg(ap, double);
    va_end(ap);

    for (int k = 0; k < n; k++)
        for (int i = 1; i <= n - k; i++)
            c[i] += a * c[i - 1];

    for (int i = 0; i <= n; i++)
        if (abs_d(c[i]) < eps) c[i] = 0;

    *res = c;
    return STATUS_OK;
}

void print_error(status_t s){
    switch (s){
        case STATUS_ERR_NULL:   fprintf(stderr, "Error: NULL pointer\n"); break;
        case STATUS_ERR_DEGREE: fprintf(stderr, "Error: degree must be in [0; INT_MAX)\n"); break;
        case STATUS_ERR_EPS:    fprintf(stderr, "Error: eps must be > 0\n"); break;
        case STATUS_ERR_ALLOC:  fprintf(stderr, "Error: failed to allocate memory\n"); break;
        default:                fprintf(stderr, "Error: unknown error\n"); break;
    }
}

void print_result(status_t s, const double* f, const double* g, int n,
                  double a, double x, double eps){
    if (s != STATUS_OK){
        print_error(s);
        return;
    }
    if (f == NULL || g == NULL){
        print_error(STATUS_ERR_NULL);
        return;
    }

    printf("g(x-a) = ");
    for (int i = 0; i <= n; i++){
        if (i > 0) printf(" + ");
        printf("%g", g[i]);
        if (n - i > 0) printf(" * (x - %g)^%d", a, n - i);
    }
    printf("\n");

    double fv = 0, gv = 0;
    poly_value(f, n, x, &fv);
    poly_value(g, n, x - a, &gv);
    printf("check (x = %g): f(x) = %g, g(x-a) = %g -> %s\n", x, fv, gv,
           (abs_d(fv - gv) < eps * (1 + abs_d(fv))) ? "OK" : "FAIL");
}

#endif
