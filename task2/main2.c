// stat() из POSIX: при -std=c99 его нужно явно включить до любых #include
#define _POSIX_C_SOURCE 200809L
#include "functions2.h"
#include <sys/stat.h>

// подсказка по использованию программы
void print_hint(void){
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  <input file> <-a|/a|-d|/d> <output file>\n");
}

// проверка флага: ровно два символа, '-' или '/', затем 'a' или 'd'
status_t parse_flag(const char* s, char* flag){
    if (s == NULL || flag == NULL) return STATUS_ERR_NULL;
    if ((s[0] != '-' && s[0] != '/') || s[1] == '\0' || s[2] != '\0') return STATUS_ERR_FLAG;
    if (s[1] != 'a' && s[1] != 'd') return STATUS_ERR_FLAG;
    *flag = s[1];
    return STATUS_OK;
}

// входной и выходной пути не должны указывать на один файл
// (иначе fopen(out, "w") сотрёт входные данные).
// Сравниваются номер устройства и inode, поэтому ловятся и разные записи
// одного пути (data.txt, ./data.txt, ../dir/data.txt), и ссылки.
status_t check_different(const char* in, const char* out){
    if (in == NULL || out == NULL) return STATUS_ERR_NULL;

    struct stat st_in, st_out;
    if (stat(out, &st_out) != 0) return STATUS_OK;   // выходного файла ещё нет
    if (stat(in, &st_in) != 0) return STATUS_OK;     // ошибку открытия входа сообщит fopen

    // на Windows (MinGW) st_ino всегда 0 - там сравниваем пути как строки
    if (st_in.st_ino == 0 || st_out.st_ino == 0)
        return (strcmp(in, out) == 0) ? STATUS_ERR_SAME_FILE : STATUS_OK;

    if (st_in.st_dev == st_out.st_dev && st_in.st_ino == st_out.st_ino)
        return STATUS_ERR_SAME_FILE;
    return STATUS_OK;
}

// полный цикл: проверка путей -> чтение -> сортировка -> запись
status_t run(const char* path_in, char flag, const char* path_out, size_t* bad_line){
    if (path_in == NULL || path_out == NULL || bad_line == NULL) return STATUS_ERR_NULL;

    status_t s = check_different(path_in, path_out);
    if (s != STATUS_OK) return s;

    FILE* in = fopen(path_in, "r");
    if (in == NULL) return STATUS_ERR_OPEN_IN;

    // сначала читаем всё: при ошибке во входе выходной файл не создаётся
    Employee* arr = NULL;
    size_t cnt = 0;
    s = read_employees(in, &arr, &cnt, bad_line);
    fclose(in);
    if (s != STATUS_OK) return s;

    s = sort_employees(arr, cnt, flag);
    if (s != STATUS_OK){
        free(arr);
        return s;
    }

    FILE* out = fopen(path_out, "w");
    if (out == NULL){
        free(arr);
        return STATUS_ERR_OPEN_OUT;
    }

    s = write_employees(out, arr, cnt);
    if (fclose(out) != 0 && s == STATUS_OK) s = STATUS_ERR_WRITE;   // данные сбрасываются на диск в fclose

    free(arr);
    return s;
}

int main(int argc, char* argv[]){
    if (argc != 4){
        print_hint();
        print_error(STATUS_ERR_ARGC, 0);
        return 1;
    }

    char flag = 0;
    status_t s = parse_flag(argv[2], &flag);
    if (s != STATUS_OK){
        print_hint();
        print_error(s, 0);
        return 1;
    }

    size_t bad_line = 0;
    s = run(argv[1], flag, argv[3], &bad_line);
    if (s != STATUS_OK){
        print_error(s, bad_line);
        return 1;
    }

    printf("Done: sorted data written to %s\n", argv[3]);
    return 0;
}
