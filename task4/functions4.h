#ifndef FUNCTIONS4_H
#define FUNCTIONS4_H

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef MAX_MEM
#define MAX_MEM (1ULL << 29)
#endif

#define TOKEN_START_CAP 32
#define FILES_START_CAP 16

typedef enum {
    STATUS_OK,
    STATUS_END,
    STATUS_ERR_NULL,
    STATUS_ERR_CNT,
    STATUS_ERR_SIZE,
    STATUS_ERR_ALLOC,
    STATUS_ERR_OVERFLOW,
    STATUS_ERR_TOO_BIG,
    STATUS_ERR_NUMBER,
    STATUS_ERR_INPUT
} status_t;

typedef struct {
    char* name;
    unsigned long long size;
    unsigned long long importance;
} file_desc_t;

typedef struct {
    file_desc_t* files;
    size_t count;
    unsigned long long total_size;
    unsigned long long total_importance;
} backup_result_t;

int is_space_c(int c){
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

void free_result(backup_result_t* r){
    if (r == NULL) return;
    free(r->files);
    r->files = NULL;
    r->count = 0;
    r->total_size = 0;
    r->total_importance = 0;
}

void free_files(file_desc_t* files, size_t n){
    if (files == NULL) return;
    for (size_t i = 0; i < n; i++)
        free(files[i].name);
    free(files);
}

status_t read_token(FILE* in, char** out){
    if (in == NULL || out == NULL) return STATUS_ERR_NULL;
    *out = NULL;

    int ch;
    while ((ch = getc(in)) != EOF && is_space_c(ch));
    if (ch == EOF) return ferror(in) ? STATUS_ERR_INPUT : STATUS_END;

    size_t cap = TOKEN_START_CAP;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if (buf == NULL) return STATUS_ERR_ALLOC;

    do {
        if (len + 1 >= cap){
            if (cap > (size_t)-1 / 2){ free(buf); return STATUS_ERR_ALLOC; }
            char* tmp = (char*)realloc(buf, cap * 2);
            if (tmp == NULL){ free(buf); return STATUS_ERR_ALLOC; }
            buf = tmp;
            cap *= 2;
        }
        buf[len++] = (char)ch;
    } while ((ch = getc(in)) != EOF && !is_space_c(ch));

    if (ch == EOF && ferror(in)){ free(buf); return STATUS_ERR_INPUT; }
    buf[len] = '\0';
    *out = buf;
    return STATUS_OK;
}

status_t parse_ull(const char* s, unsigned long long* out){
    if (s == NULL || out == NULL) return STATUS_ERR_NULL;
    if (s[0] == '\0') return STATUS_ERR_NUMBER;

    unsigned long long v = 0;
    for (size_t i = 0; s[i] != '\0'; i++){
        if (s[i] < '0' || s[i] > '9') return STATUS_ERR_NUMBER;
        unsigned long long d = (unsigned long long)(s[i] - '0');
        if (v > (ULLONG_MAX - d) / 10) return STATUS_ERR_NUMBER;
        v = v * 10 + d;
    }
    *out = v;
    return STATUS_OK;
}

status_t read_number(FILE* in, unsigned long long* out){
    if (in == NULL || out == NULL) return STATUS_ERR_NULL;
    char* tok = NULL;
    status_t s = read_token(in, &tok);
    if (s == STATUS_END) return STATUS_ERR_INPUT;
    if (s != STATUS_OK) return s;
    s = parse_ull(tok, out);
    free(tok);
    return s;
}

status_t read_files(FILE* in, size_t n, file_desc_t** files, size_t* bad, int* field){
    if (in == NULL || files == NULL || bad == NULL || field == NULL) return STATUS_ERR_NULL;
    if (n < 1) return STATUS_ERR_CNT;
    *files = NULL;
    *bad = 0;
    *field = 0;

    size_t cap = (n < FILES_START_CAP) ? n : FILES_START_CAP;
    file_desc_t* arr = (file_desc_t*)calloc(cap, sizeof(file_desc_t));
    if (arr == NULL) return STATUS_ERR_ALLOC;

    for (size_t i = 0; i < n; i++){
        *bad = i + 1;
        if (i == cap){
            size_t new_cap = (cap > n / 2) ? n : cap * 2;
            if (new_cap > (size_t)-1 / sizeof(file_desc_t)){ free_files(arr, i); return STATUS_ERR_ALLOC; }
            file_desc_t* tmp = (file_desc_t*)realloc(arr, new_cap * sizeof(file_desc_t));
            if (tmp == NULL){ free_files(arr, i); return STATUS_ERR_ALLOC; }
            arr = tmp;
            cap = new_cap;
        }
        arr[i].name = NULL;

        char* name = NULL;
        *field = 1;
        status_t s = read_token(in, &name);
        if (s == STATUS_END) s = STATUS_ERR_INPUT;
        if (s != STATUS_OK){ free_files(arr, i); return s; }
        arr[i].name = name;

        *field = 2;
        s = read_number(in, &arr[i].size);
        if (s == STATUS_OK && arr[i].size < 1) s = STATUS_ERR_SIZE;
        if (s != STATUS_OK){ free_files(arr, i + 1); return s; }

        *field = 3;
        s = read_number(in, &arr[i].importance);
        if (s != STATUS_OK){ free_files(arr, i + 1); return s; }
    }
    *files = arr;
    return STATUS_OK;
}

status_t build_result(const file_desc_t* files, size_t n, const unsigned char* marks,
                      backup_result_t* result){
    size_t cnt = 0;
    for (size_t i = 0; i < n; i++)
        if (marks[i]) cnt++;
    if (cnt == 0) return STATUS_OK;

    file_desc_t* chosen = (file_desc_t*)malloc(cnt * sizeof(file_desc_t));
    if (chosen == NULL) return STATUS_ERR_ALLOC;

    unsigned long long ts = 0, ti = 0;
    size_t idx = 0;
    for (size_t i = 0; i < n; i++){
        if (!marks[i]) continue;
        if (files[i].importance > ULLONG_MAX - ti || files[i].size > ULLONG_MAX - ts){
            free(chosen);
            return STATUS_ERR_OVERFLOW;
        }
        chosen[idx++] = files[i];
        ts += files[i].size;
        ti += files[i].importance;
    }
    result->files = chosen;
    result->count = cnt;
    result->total_size = ts;
    result->total_importance = ti;
    return STATUS_OK;
}

status_t select_files(const file_desc_t* files, size_t n, unsigned long long cap,
                      backup_result_t* result){
    if (files == NULL || result == NULL) return STATUS_ERR_NULL;
    result->files = NULL;
    result->count = 0;
    result->total_size = 0;
    result->total_importance = 0;
    if (n < 1) return STATUS_ERR_CNT;

    unsigned long long sum = 0;
    int saturated = 0;
    for (size_t i = 0; i < n; i++){
        if (files[i].size < 1) return STATUS_ERR_SIZE;
        if (sum > ULLONG_MAX - files[i].size) saturated = 1;
        else sum += files[i].size;
    }

    unsigned char* marks = (unsigned char*)calloc(n, sizeof(unsigned char));
    if (marks == NULL) return STATUS_ERR_ALLOC;

    if (!saturated && sum <= cap){
        for (size_t i = 0; i < n; i++) marks[i] = 1;
        status_t s = build_result(files, n, marks, result);
        free(marks);
        return s;
    }

    if (cap >= MAX_MEM / sizeof(unsigned long long)){ free(marks); return STATUS_ERR_TOO_BIG; }
    unsigned long long w_cnt = cap + 1;
    unsigned long long dp_bytes = w_cnt * sizeof(unsigned long long);
    if ((unsigned long long)n > (MAX_MEM * 8ULL) / w_cnt){ free(marks); return STATUS_ERR_TOO_BIG; }
    unsigned long long bits = (unsigned long long)n * w_cnt;
    unsigned long long take_bytes = (bits + 7) / 8;
    if (take_bytes > MAX_MEM - dp_bytes){ free(marks); return STATUS_ERR_TOO_BIG; }

    unsigned long long* dp = (unsigned long long*)calloc((size_t)w_cnt, sizeof(unsigned long long));
    unsigned char* take = (unsigned char*)calloc((size_t)take_bytes, sizeof(unsigned char));
    if (dp == NULL || take == NULL){
        free(dp); free(take); free(marks);
        return STATUS_ERR_ALLOC;
    }

    for (size_t i = 0; i < n; i++){
        unsigned long long fs = files[i].size;
        unsigned long long fi = files[i].importance;
        if (fs > cap) continue;

        for (unsigned long long w = cap; w >= fs; w--){
            unsigned long long prev = dp[w - fs];
            if (fi > ULLONG_MAX - prev){
                free(dp); free(take); free(marks);
                return STATUS_ERR_OVERFLOW;
            }
            if (prev + fi > dp[w]){
                dp[w] = prev + fi;
                unsigned long long bit = (unsigned long long)i * w_cnt + w;
                take[bit >> 3] = (unsigned char)(take[bit >> 3] | (1u << (bit & 7)));
            }
        }
    }

    unsigned long long w = cap;
    for (size_t i = n; i-- > 0;){
        unsigned long long bit = (unsigned long long)i * w_cnt + w;
        if (take[bit >> 3] & (1u << (bit & 7))){
            marks[i] = 1;
            w -= files[i].size;
        }
    }

    status_t s = build_result(files, n, marks, result);
    free(dp); free(take); free(marks);
    return s;
}

void print_error(status_t s, size_t bad, int field){
    const char* f = (field == 1) ? "name" : (field == 2) ? "size" : (field == 3) ? "importance" : "";
    switch (s){
        case STATUS_OK:
        case STATUS_END:          break;
        case STATUS_ERR_NULL:     fprintf(stderr, "Error: NULL pointer\n"); break;
        case STATUS_ERR_CNT:      fprintf(stderr, "Error: n must be >= 1\n"); break;
        case STATUS_ERR_SIZE:     fprintf(stderr, "Error: file %zu: size must be >= 1\n", bad); break;
        case STATUS_ERR_ALLOC:    fprintf(stderr, "Error: failed to allocate memory\n"); break;
        case STATUS_ERR_OVERFLOW: fprintf(stderr, "Error: total importance does not fit in unsigned long long\n"); break;
        case STATUS_ERR_TOO_BIG:  fprintf(stderr, "Error: capacity is too large for the table limit (%llu bytes)\n", (unsigned long long)MAX_MEM); break;
        case STATUS_ERR_NUMBER:
            if (bad > 0) fprintf(stderr, "Error: file %zu: invalid %s (non-negative integer expected)\n", bad, f);
            else fprintf(stderr, "Error: invalid number in the first line\n");
            break;
        case STATUS_ERR_INPUT:
            if (bad > 0) fprintf(stderr, "Error: unexpected end of input at file %zu (%s)\n", bad, f);
            else fprintf(stderr, "Error: failed to read n and capacity\n");
            break;
        default:                  fprintf(stderr, "Error: unknown error\n"); break;
    }
}

status_t print_result(FILE* out, const backup_result_t* r){
    if (out == NULL || r == NULL) return STATUS_ERR_NULL;
    if (fprintf(out, "TOTAL_SIZE %llu\nTOTAL_IMPORTANCE %llu\nFILES %zu\n",
                r->total_size, r->total_importance, r->count) < 0)
        return STATUS_ERR_INPUT;
    for (size_t i = 0; i < r->count; i++)
        if (fprintf(out, "%s\n", r->files[i].name) < 0) return STATUS_ERR_INPUT;
    return STATUS_OK;
}

#endif
