#include "functions7.h"

int main(void){
    norm_result_t* r = NULL;
    status_t s;

    double c0[] = { 1,  2,  3};
    double c1[] = { 3, -1,  0};
    double c2[] = { 0, -3,  1};
    double c3[] = {-2,  2, -2};
    vector_t v[4];
    v[0].coords = c0;
    v[1].coords = c1;
    v[2].coords = c2;
    v[3].coords = c3;

    double p1 = 1.0, p2 = 2.0;
    double a[] = {2, 1, 0,
                  1, 2, 1,
                  0, 1, 2};
    matrix_params_t mp;
    s = init_matrix_params(&mp, a, 3);
    if (s != STATUS_OK){
        print_error(s);
        return 1;
    }

    const char* names4[] = {"inf-norm", "p-norm, p=1", "p-norm, p=2", "A-norm"};
    printf("1) 4 vectors in R^3, 4 norms\n");
    s = longest_vectors(&r, 3, 4, 4, v[0], v[1], v[2], v[3],
                        norm_inf, (const void*)NULL,
                        norm_p, (const void*)&p1,
                        norm_p, (const void*)&p2,
                        norm_a, (const void*)&mp);
    print_results(s, v, 3, names4, r, 4);
    free_results(r, 4);
    r = NULL;

    const char* names_a[] = {"A-norm"};
    printf("\n2) vectors #2 and #3 only, A-norm\n");
    s = longest_vectors(&r, 3, 2, 1, v[1], v[2], norm_a, (const void*)&mp);
    print_results(s, &v[1], 3, names_a, r, 1);
    free_results(r, 1);
    r = NULL;

    const char* names_p[] = {"p-norm"};
    double bad_p = 0.5;
    printf("\n3) p = 0.5\n");
    s = longest_vectors(&r, 3, 4, 1, v[0], v[1], v[2], v[3],
                        norm_p, (const void*)&bad_p);
    print_results(s, v, 3, names_p, r, 1);
    free_results(r, 1);
    r = NULL;

    double b[] = {1, 2, 0,
                  2, 1, 0,
                  0, 0, 1};
    matrix_params_t bad;
    printf("\n4) matrix [[1 2 0] [2 1 0] [0 0 1]]\n");
    s = init_matrix_params(&bad, b, 3);
    if (s != STATUS_OK) print_error(s);
    else printf("matrix accepted\n");

    const char* names_i[] = {"inf-norm"};
    printf("\n5) n = 0\n");
    s = longest_vectors(&r, 0, 4, 1, v[0], v[1], v[2], v[3],
                        norm_inf, (const void*)NULL);
    print_results(s, v, 0, names_i, r, 1);
    free_results(r, 1);

    return 0;
}
