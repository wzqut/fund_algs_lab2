#ifndef FUNCTIONS3_H
#define FUNCTIONS3_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    STATUS_OK,
    STATUS_ERR_NULL,
    STATUS_ERR_EMPTY,
    STATUS_ERR_CNT,
    STATUS_ERR_FILE,
    STATUS_ERR_READ,
    STATUS_ERR_ALLOC,
    STATUS_ERR_OVERFLOW
} status_t;

typedef struct {
    size_t line;
    size_t pos;
} Pos;

typedef struct {
    const char* path;
    Pos* arr;
    size_t cnt;
} FileRes;

size_t len_s(const char* s){
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

status_t read_file(const char* path, char** text, size_t* len){
    if (path == NULL || text == NULL || len == NULL) return STATUS_ERR_NULL;

    FILE* f = fopen(path, "rb");
    if (f == NULL) return STATUS_ERR_FILE;

    size_t cap = 64, n = 0;
    char* buf = (char*)malloc(cap);
    if (buf == NULL){
        fclose(f);
        return STATUS_ERR_ALLOC;
    }

    int c;
    while ((c = fgetc(f)) != EOF){
        if (n + 1 >= cap){
            if (cap > SIZE_MAX / 2){
                free(buf);
                fclose(f);
                return STATUS_ERR_OVERFLOW;
            }
            cap *= 2;
            char* t = (char*)realloc(buf, cap);
            if (t == NULL){
                free(buf);
                fclose(f);
                return STATUS_ERR_ALLOC;
            }
            buf = t;
        }
        buf[n++] = (char)c;
    }

    if (ferror(f)){
        free(buf);
        fclose(f);
        return STATUS_ERR_READ;
    }
    fclose(f);

    *text = buf;
    *len = n;
    return STATUS_OK;
}

int match_at(const char* t, size_t n, size_t i, const char* sub, size_t m){
    if (i > n || m > n - i) return 0;
    for (size_t j = 0; j < m; j++)
        if (t[i + j] != sub[j]) return 0;
    return 1;
}

status_t search_text(const char* t, size_t n, const char* sub, size_t m, Pos** arr, size_t* cnt){
    if (t == NULL || sub == NULL || arr == NULL || cnt == NULL) return STATUS_ERR_NULL;
    if (m == 0) return STATUS_ERR_EMPTY;

    Pos* a = NULL;
    size_t cap = 0, k = 0;
    size_t line = 1, pos = 1;

    for (size_t i = 0; i < n; i++){
        if (match_at(t, n, i, sub, m)){
            if (k == cap){
                size_t new_cap = (cap == 0) ? 4 : cap * 2;
                if (cap > SIZE_MAX / 2 / sizeof(Pos)){
                    free(a);
                    return STATUS_ERR_OVERFLOW;
                }
                Pos* tmp = (Pos*)realloc(a, sizeof(Pos) * new_cap);
                if (tmp == NULL){
                    free(a);
                    return STATUS_ERR_ALLOC;
                }
                a = tmp;
                cap = new_cap;
            }
            a[k].line = line;
            a[k].pos = pos;
            k++;
        }

        if (t[i] == '\n'){
            line++;
            pos = 1;
        } else {
            pos++;
        }
    }

    *arr = a;
    *cnt = k;
    return STATUS_OK;
}

void free_res(FileRes* r, int cnt){
    if (r == NULL) return;
    for (int i = 0; i < cnt; i++) free(r[i].arr);
    free(r);
}

status_t find_all(FileRes** res, const char* sub, int cnt, ...){
    if (res == NULL || sub == NULL) return STATUS_ERR_NULL;
    if (sub[0] == '\0') return STATUS_ERR_EMPTY;
    if (cnt < 1) return STATUS_ERR_CNT;

    size_t m = len_s(sub);

    FileRes* r = (FileRes*)calloc((size_t)cnt, sizeof(FileRes));
    if (r == NULL) return STATUS_ERR_ALLOC;

    va_list ap;
    va_start(ap, cnt);

    for (int i = 0; i < cnt; i++){
        const char* path = va_arg(ap, const char*);
        r[i].path = path;

        char* text = NULL;
        size_t n = 0;
        status_t s = read_file(path, &text, &n);
        if (s == STATUS_OK){
            s = search_text(text, n, sub, m, &r[i].arr, &r[i].cnt);
            free(text);
        }

        if (s != STATUS_OK){
            va_end(ap);
            free_res(r, cnt);
            return s;
        }
    }

    va_end(ap);
    *res = r;
    return STATUS_OK;
}

void print_error(status_t s){
    switch (s){
        case STATUS_ERR_NULL:     fprintf(stderr, "Error: NULL pointer\n"); break;
        case STATUS_ERR_EMPTY:    fprintf(stderr, "Error: empty substring\n"); break;
        case STATUS_ERR_CNT:      fprintf(stderr, "Error: count of files must be >= 1\n"); break;
        case STATUS_ERR_FILE:     fprintf(stderr, "Error: cannot open file\n"); break;
        case STATUS_ERR_READ:     fprintf(stderr, "Error: cannot read file\n"); break;
        case STATUS_ERR_ALLOC:    fprintf(stderr, "Error: failed to allocate memory\n"); break;
        case STATUS_ERR_OVERFLOW: fprintf(stderr, "Error: size overflow\n"); break;
        default:                  fprintf(stderr, "Error: unknown error\n"); break;
    }
}

void print_res(status_t s, const FileRes* r, int cnt){
    if (s != STATUS_OK){
        print_error(s);
        return;
    }
    if (r == NULL) return;
    for (int i = 0; i < cnt; i++){
        printf("%s:", r[i].path);
        if (r[i].cnt == 0) printf(" not found");
        for (size_t j = 0; j < r[i].cnt; j++)
            printf(" %zu:%zu", r[i].arr[j].line, r[i].arr[j].pos);
        printf("\n");
    }
}

#endif
