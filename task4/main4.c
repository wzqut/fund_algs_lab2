#include "functions4.h"

int main(void){
    unsigned long long n_ull = 0, cap = 0;
    status_t s = read_number(stdin, &n_ull);
    if (s == STATUS_OK) s = read_number(stdin, &cap);
    if (s == STATUS_OK && n_ull < 1) s = STATUS_ERR_CNT;
    if (s == STATUS_OK && n_ull > (unsigned long long)((size_t)-1)) s = STATUS_ERR_ALLOC;
    if (s != STATUS_OK){
        print_error(s, 0, 0);
        return 1;
    }

    size_t bad = 0;
    int field = 0;
    file_desc_t* files = NULL;
    s = read_files(stdin, (size_t)n_ull, &files, &bad, &field);
    if (s != STATUS_OK){
        print_error(s, bad, field);
        return 1;
    }

    backup_result_t result;
    s = select_files(files, (size_t)n_ull, cap, &result);
    if (s == STATUS_OK && print_result(stdout, &result) != STATUS_OK)
        s = STATUS_ERR_INPUT;
    if (s != STATUS_OK) print_error(s, 0, 0);

    free_result(&result);
    free_files(files, (size_t)n_ull);
    return (s == STATUS_OK) ? 0 : 1;
}
