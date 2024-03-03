#include <stdio.h>
#include <string.h>

typedef struct {
    char* val;
    size_t len;
    size_t alloc_cur;
} string;

void string_init(string* str);
void string_add_format(string* str, char* format, ...);
void string_add(string* str, const char* val);
void string_add_n(string* str, const char* val, size_t len);
void string_finalize(string* str);