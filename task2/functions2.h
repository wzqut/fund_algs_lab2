#ifndef FUNCTIONS2_H
#define FUNCTIONS2_H

#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// точность сравнения зарплат; можно переопределить при сборке: -DEPS=1e-6
#ifndef EPS
#define EPS 1e-9
#endif

#define NAME_LEN 64    // буфер для имени / фамилии: до 63 букв + '\0'
#define LINE_LEN 256   // буфер для одной строки файла (с '\0')
#define START_CAP 16   // начальная ёмкость динамического массива

typedef enum {
    STATUS_OK,              // успех
    STATUS_ERR_ARGC,        // неверное число аргументов
    STATUS_ERR_NULL,        // передан NULL / некорректные параметры
    STATUS_ERR_FLAG,        // неизвестный флаг
    STATUS_ERR_SAME_FILE,   // входной и выходной файл совпадают
    STATUS_ERR_OPEN_IN,     // не удалось открыть входной файл
    STATUS_ERR_OPEN_OUT,    // не удалось открыть выходной файл
    STATUS_ERR_READ,        // ошибка чтения
    STATUS_ERR_WRITE,       // ошибка записи
    STATUS_ERR_LONG_LINE,   // строка файла длиннее LINE_LEN - 1 символов
    STATUS_ERR_FIELDS,      // в строке не 4 поля
    STATUS_ERR_ID,          // неверный id
    STATUS_ERR_NAME,        // неверное имя / фамилия
    STATUS_ERR_SALARY,      // неверная зарплата
    STATUS_ERR_ALLOC,       // не удалось выделить память
    STATUS_ERR_OVERFLOW,    // переполнение размера массива
    STATUS_END              // конец файла (не ошибка, внутренний статус)
} status_t;

typedef struct {
    unsigned int id;
    char name[NAME_LEN];
    char surname[NAME_LEN];
    double salary;
} Employee;


// ---------- вспомогательные проверки символов ----------

// символ - цифра
int is_digit_c(char c){
    return c >= '0' && c <= '9';
}

// символ - латинская буква
int is_alpha_c(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// символ - разделитель полей (пробел или табуляция)
int is_space_c(char c){
    return c == ' ' || c == '\t';
}


// ---------- разбор полей ----------

// id: только цифры, не больше UINT_MAX
status_t parse_id(const char* s, unsigned int* res){
    if (s == NULL || res == NULL) return STATUS_ERR_NULL;
    if (s[0] == '\0') return STATUS_ERR_ID;

    unsigned int v = 0;
    for (size_t i = 0; s[i] != '\0'; i++){
        if (!is_digit_c(s[i])) return STATUS_ERR_ID;   // в т.ч. '-' и '+'
        unsigned int d = (unsigned int)(s[i] - '0');
        if (v > (UINT_MAX - d) / 10) return STATUS_ERR_ID;   // переполнение
        v = v * 10 + d;
    }
    *res = v;
    return STATUS_OK;
}

// имя / фамилия: непустая строка из латинских букв, короче NAME_LEN; копируется в out
status_t parse_name(const char* s, char* out){
    if (s == NULL || out == NULL) return STATUS_ERR_NULL;
    if (s[0] == '\0') return STATUS_ERR_NAME;

    size_t i = 0;
    for (; s[i] != '\0'; i++){
        if (!is_alpha_c(s[i])) return STATUS_ERR_NAME;   // отсекает и кириллицу
        if (i + 1 >= NAME_LEN) return STATUS_ERR_NAME;   // слишком длинное
        out[i] = s[i];
    }
    out[i] = '\0';
    return STATUS_OK;
}

// зарплата: цифры [ '.' цифры ], например 1500 или 1500.50
// не допускаются: знак, запятая, экспонента, ".5", "15.", inf, nan
status_t parse_salary(const char* s, double* res){
    if (s == NULL || res == NULL) return STATUS_ERR_NULL;

    size_t i = 0;
    if (!is_digit_c(s[i])) return STATUS_ERR_SALARY;
    while (is_digit_c(s[i])) i++;

    if (s[i] == '.'){
        i++;
        if (!is_digit_c(s[i])) return STATUS_ERR_SALARY;
        while (is_digit_c(s[i])) i++;
    }
    if (s[i] != '\0') return STATUS_ERR_SALARY;

    // формат уже проверен, strtod только переводит строку в число
    double v = strtod(s, NULL);
    if (v > DBL_MAX) return STATUS_ERR_SALARY;   // слишком большое число (inf)

    *res = v;
    return STATUS_OK;
}


// ---------- разбор строки ----------

// делит строку по пробелам / табуляциям (строка меняется: после слов ставится '\0')
// tok получает до max слов, cnt - число найденных слов (max + 1, если слов больше)
status_t split_line(char* line, char* tok[], int max, int* cnt){
    if (line == NULL || tok == NULL || cnt == NULL || max < 0) return STATUS_ERR_NULL;

    int k = 0;
    size_t i = 0;
    while (line[i] != '\0'){
        while (is_space_c(line[i])) i++;
        if (line[i] == '\0') break;

        if (k == max){   // слов больше, чем нужно
            *cnt = k + 1;
            return STATUS_OK;
        }
        tok[k++] = &line[i];

        while (line[i] != '\0' && !is_space_c(line[i])) i++;
        if (line[i] != '\0') line[i++] = '\0';
    }
    *cnt = k;
    return STATUS_OK;
}

// "id имя фамилия зарплата" -> Employee
status_t parse_employee(char* line, Employee* e){
    if (line == NULL || e == NULL) return STATUS_ERR_NULL;

    char* tok[4];
    int cnt = 0;
    status_t s = split_line(line, tok, 4, &cnt);
    if (s != STATUS_OK) return s;
    if (cnt != 4) return STATUS_ERR_FIELDS;

    s = parse_id(tok[0], &e->id);
    if (s != STATUS_OK) return s;
    s = parse_name(tok[1], e->name);
    if (s != STATUS_OK) return s;
    s = parse_name(tok[2], e->surname);
    if (s != STATUS_OK) return s;
    return parse_salary(tok[3], &e->salary);
}

// строка состоит только из пробелов / табуляций (или пустая)
int is_blank(const char* s){
    if (s == NULL) return 1;
    for (size_t i = 0; s[i] != '\0'; i++)
        if (!is_space_c(s[i])) return 0;
    return 1;
}


// ---------- чтение из потока ----------

// читает одну строку в buf (без '\n'; '\r' от Windows-файлов отбрасывается)
// STATUS_END - файл закончился, STATUS_ERR_LONG_LINE - строка не помещается в буфер
status_t read_line(FILE* in, char* buf, size_t size){
    if (in == NULL || buf == NULL || size == 0) return STATUS_ERR_NULL;

    int c = fgetc(in);
    if (c == EOF) return ferror(in) ? STATUS_ERR_READ : STATUS_END;

    size_t n = 0;
    while (c != EOF && c != '\n'){
        if (c != '\r'){
            if (n + 1 >= size) return STATUS_ERR_LONG_LINE;
            buf[n++] = (char)c;
        }
        c = fgetc(in);
    }
    if (ferror(in)) return STATUS_ERR_READ;

    buf[n] = '\0';
    return STATUS_OK;
}

// читает всех сотрудников в динамический массив (*arr нужно освободить через free)
// пустые строки пропускаются; при ошибке в данных *bad_line - номер строки (с 1)
status_t read_employees(FILE* in, Employee** arr, size_t* cnt, size_t* bad_line){
    if (in == NULL || arr == NULL || cnt == NULL || bad_line == NULL) return STATUS_ERR_NULL;

    size_t cap = START_CAP, n = 0;
    Employee* a = (Employee*)malloc(sizeof(Employee) * cap);
    if (a == NULL) return STATUS_ERR_ALLOC;

    char line[LINE_LEN];
    size_t line_no = 0;
    status_t s = STATUS_OK;

    while (s == STATUS_OK){
        line_no++;
        s = read_line(in, line, sizeof(line));
        if (s != STATUS_OK) break;            // конец файла или ошибка
        if (is_blank(line)) continue;

        if (n == cap){                        // массив заполнен - увеличиваем в 2 раза
            if (cap > SIZE_MAX / 2 / sizeof(Employee)){
                s = STATUS_ERR_OVERFLOW;
                break;
            }
            Employee* t = (Employee*)realloc(a, sizeof(Employee) * cap * 2);
            if (t == NULL){
                s = STATUS_ERR_ALLOC;
                break;
            }
            a = t;
            cap *= 2;
        }

        s = parse_employee(line, &a[n]);
        if (s == STATUS_OK) n++;
    }

    if (s != STATUS_END){                     // ошибка: массив не отдаём
        free(a);
        *bad_line = line_no;
        return s;
    }

    *arr = a;
    *cnt = n;
    return STATUS_OK;
}


// ---------- сортировка ----------

// сравнение вещественных с точностью eps: -1 (a < b), 0 (равны), 1 (a > b)
int cmp_double(double a, double b, double eps){
    if (a - b > eps) return 1;
    if (b - a > eps) return -1;
    return 0;
}

// сравнение беззнаковых без вычитания (a - b для unsigned дало бы переполнение)
int cmp_uint(unsigned int a, unsigned int b){
    return (a > b) - (a < b);
}

// компаратор для qsort по возрастанию: зарплата -> фамилия -> имя -> id
// сигнатура qsort не позволяет передать eps параметром, поэтому
// компаратор передаёт константу EPS в cmp_double, где она уже параметр
int cmp_asc(const void* pa, const void* pb){
    const Employee* a = (const Employee*)pa;
    const Employee* b = (const Employee*)pb;

    int r = cmp_double(a->salary, b->salary, EPS);
    if (r != 0) return r;

    r = strcmp(a->surname, b->surname);
    if (r != 0) return r;

    r = strcmp(a->name, b->name);
    if (r != 0) return r;

    return cmp_uint(a->id, b->id);
}

// компаратор по убыванию: тот же порядок, но аргументы поменяны местами
int cmp_desc(const void* pa, const void* pb){
    return cmp_asc(pb, pa);
}

// сортировка стандартной qsort; flag: 'a' - по возрастанию, 'd' - по убыванию
status_t sort_employees(Employee* arr, size_t cnt, char flag){
    if (arr == NULL && cnt > 0) return STATUS_ERR_NULL;
    if (flag != 'a' && flag != 'd') return STATUS_ERR_FLAG;
    if (cnt < 2) return STATUS_OK;

    qsort(arr, cnt, sizeof(Employee), (flag == 'a') ? cmp_asc : cmp_desc);
    return STATUS_OK;
}


// ---------- вывод ----------

// запись сотрудников в поток в том же формате, что и вход
// %.15g: зарплата без лишних нулей и без потери знаков (1500.5, 0.001, 1500)
status_t write_employees(FILE* out, const Employee* arr, size_t cnt){
    if (out == NULL || (arr == NULL && cnt > 0)) return STATUS_ERR_NULL;

    for (size_t i = 0; i < cnt; i++)
        if (fprintf(out, "%u %s %s %.15g\n", arr[i].id, arr[i].name,
                    arr[i].surname, arr[i].salary) < 0)
            return STATUS_ERR_WRITE;
    return STATUS_OK;
}

// сообщение об ошибке в stderr; bad_line - номер строки для ошибок данных
void print_error(status_t s, size_t bad_line){
    switch (s){
        case STATUS_OK:
        case STATUS_END:           break;
        case STATUS_ERR_ARGC:      fprintf(stderr, "Error: wrong number of arguments\n"); break;
        case STATUS_ERR_NULL:      fprintf(stderr, "Error: invalid parameters\n"); break;
        case STATUS_ERR_FLAG:      fprintf(stderr, "Error: unknown flag (expected -a, /a, -d or /d)\n"); break;
        case STATUS_ERR_SAME_FILE: fprintf(stderr, "Error: input and output files must be different\n"); break;
        case STATUS_ERR_OPEN_IN:   fprintf(stderr, "Error: cannot open input file\n"); break;
        case STATUS_ERR_OPEN_OUT:  fprintf(stderr, "Error: cannot open output file\n"); break;
        case STATUS_ERR_READ:      fprintf(stderr, "Error: cannot read input file\n"); break;
        case STATUS_ERR_WRITE:     fprintf(stderr, "Error: cannot write output file\n"); break;
        case STATUS_ERR_LONG_LINE:
            fprintf(stderr, "Error: line %zu is too long (max %d characters)\n", bad_line, LINE_LEN - 1);
            break;
        case STATUS_ERR_FIELDS:
            fprintf(stderr, "Error: line %zu: expected 4 fields: id name surname salary\n", bad_line);
            break;
        case STATUS_ERR_ID:
            fprintf(stderr, "Error: line %zu: id must be an integer from 0 to %u\n", bad_line, UINT_MAX);
            break;
        case STATUS_ERR_NAME:
            fprintf(stderr, "Error: line %zu: name and surname must be 1..%d latin letters\n", bad_line, NAME_LEN - 1);
            break;
        case STATUS_ERR_SALARY:
            fprintf(stderr, "Error: line %zu: salary must be a non-negative number like 1500 or 1500.50\n", bad_line);
            break;
        case STATUS_ERR_ALLOC:     fprintf(stderr, "Error: failed to allocate memory\n"); break;
        case STATUS_ERR_OVERFLOW:  fprintf(stderr, "Error: too many employees\n"); break;
        default:                   fprintf(stderr, "Error: unknown error\n"); break;
    }
}

#endif
