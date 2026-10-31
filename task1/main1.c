#include "functions1.h"

void print_hint(void){
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  <-l|-r|-u|-n> <string>\n");
    fprintf(stderr, "  -c <string> <seed> [string ...]\n");
}

int is_valid_flag(const char* f, char* out){
    if (f == NULL || out == NULL) return 0;
    if (f[0] != '-' || f[1] == '\0' || f[2] != '\0') return 0;

    char c = f[1];
    if (c == 'l' || c == 'r' || c == 'u' || c == 'n' || c == 'c'){
        *out = c;
        return 1;
    }
    return 0;
}

status_t run_str_flag(char flag, const char* s){
    char* r = NULL;
    status_t st;
    const char* name;

    switch (flag){
        case 'r': st = str_reverse(s, &r);    name = "Reversed"; break;
        case 'u': st = str_odd_upper(s, &r);  name = "Odd upper"; break;
        case 'n': st = str_sort_types(s, &r); name = "Digits, letters, others"; break;
        default: return STATUS_ERR_FLAG;
    }

    if (st == STATUS_OK) print_str(name, r);
    free(r);
    return st;
}

status_t run_concat(int argc, char* argv[]){
    unsigned int seed = 0;
    status_t st = parse_uint(argv[3], &seed);
    if (st != STATUS_OK) return st;

    int cnt = argc - 3;
    const char** strs = (const char**)malloc(sizeof(const char*) * (size_t)cnt);
    if (strs == NULL) return STATUS_ERR_ALLOC;

    strs[0] = argv[2];
    for (int i = 4; i < argc; i++) strs[i - 3] = argv[i];

    char* r = NULL;
    st = str_concat_random(strs, cnt, seed, &r);
    if (st == STATUS_OK){
        printf("Seed: %u\n", seed);
        print_str("Concat", r);
    }

    free(r);
    free(strs);
    return st;
}

int main(int argc, char* argv[]){
    if (argc < 3){
        print_hint();
        print_error(STATUS_ERR_ARGC);
        return 1;
    }

    char flag = 0;
    if (!is_valid_flag(argv[1], &flag)){
        print_hint();
        print_error(STATUS_ERR_FLAG);
        return 1;
    }

    if ((flag != 'c' && argc != 3) || (flag == 'c' && argc < 4)){
        print_hint();
        print_error(STATUS_ERR_ARGC);
        return 1;
    }

    status_t s = STATUS_OK;

    if (flag == 'l'){
        size_t len = 0;
        s = str_len(argv[2], &len);
        if (s == STATUS_OK) print_len(argv[2], len);
    } else if (flag == 'c'){
        s = run_concat(argc, argv);
    } else {
        s = run_str_flag(flag, argv[2]);
    }

    if (s != STATUS_OK){
        print_error(s);
        return 1;
    }
    return 0;
}
