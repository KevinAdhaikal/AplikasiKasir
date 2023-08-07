#include "str.h"

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define UINT64_MAX_STRLEN 20

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define STR_TAIL_P(str) ((str)->value + (str)->length)

/**
 * Calculates the length of the given string.
 */
static int64_t str_get_len(const char *s)
{
    const char *n = memchr(s, 0, INT64_MAX);
    if (n) {
        return (int64_t) (n - s);
    }

    return INT64_MAX;
}

/**
 * Compares 2 strings.
 */
static int str_memncmp(const char *a, int64_t a_len, const char *b, int64_t b_len)
{
    int result = memcmp(a, b, MIN(a_len, b_len));
    if (!result) {
        result = a_len - b_len;
    }

    return result;
}

static char *str_memnstr(char *s, int64_t s_len, const char *needle, int64_t needle_len)
{
    if (s_len == 0 || needle_len == 0) {
        /* All strings contain an empty string */
        return s;
    }

    if (s_len >= needle_len) {
        if (needle_len == 1) {
            return memchr(s, *needle, s_len);
        } else {
            const char *end = s + s_len;

            /* Find the first starting character of needle in the haystack. */
            s = memchr(s, *needle, end - s);

            while (s != NULL && s + needle_len <= end) {
                /* Compare and check if we have found the needle */
                if (memcmp(s, needle, needle_len) == 0) {
                    return s;
                }

                /* Not the needle. Check again */
                s = memchr(s + needle_len, *needle, end - s + needle_len);
            }
        }
    }

    /* Needle was not found */
    return NULL;
}

bool str_init_size(Str *str, int64_t size)
{
    char *mem = malloc(sizeof(char) * size);
    if (mem) {
        str->value = mem;
        str->size = size;
        str->length = 0;
        mem[0] = '\0';
        return true;
    }

    return false;
}

void str_finalize(Str *str)
{
    if (str && str->value) {
        free(str->value);
        str->value = NULL;
        str->size = 0;
        str->length = 0;
    }
}

bool str_set_size(Str *str, int64_t size)
{
    char *mem = realloc(str->value, sizeof(char) * size);
    if (mem) {
        str->value = mem;
        str->size = size;

        if (str->length >= size) {
            str->length = size - 1;
            str->value[str->length] = '\0';
        }

        return true;
    }

    return false;
}

bool str_set_length(Str *str, int64_t length)
{
    if (str_ensure_capacity(str, length + 1)) {
        if (length > str->length) {
            memset(STR_TAIL_P(str), 0, length - str->length);
        }

        str->value[length] = '\0';
        str->length = length;
        return true;
    }

    return false;
}

bool str_ensure_capacity(Str *str, int64_t min_size)
{
    if (min_size > str->size) {
        int64_t size = str->size * 2;

        if (min_size > size) {
            size = min_size;
        }

        return str_set_size(str, size);
    }

    /* Allocated memory is large enough */
    return true;
}

bool str_copy(const Str *source, Str *destination)
{
    if (str_init_size(destination, source->length + 1)) {
        memcpy(destination->value, source->value, source->length);
        destination->value[source->length] = '\0';
        destination->length = source->length;
        return true;
    }

    return false;
}

int str_compare(const Str *a, const Str *b)
{
    return str_memncmp(a->value, a->length, b->value, b->length);
}

int str_compare_str(const Str *str, const char *s, int64_t length)
{
    if (length < 0) {
        length = str_get_len(s);
    }

    return str_memncmp(str->value, str->length, s, length);
}

bool str_equals_str(const Str *a, const char *s, int64_t length)
{
    if (length < 0) {
        length = str_get_len(s);
    }

    return a->length == length && memcmp(a->value, s, length) == 0;
}

int64_t str_indexof_str(const Str *str, const char *substr, int64_t length)
{
    if (length < 0) {
        length = str_get_len(substr);
    }

    const char *r = str_memnstr(str->value, str->length, substr, length);
    if (r) {
        return (int64_t) (r - str->value);
    } else {
        return -1;
    }
}

bool str_contains_str(const Str *str, const char *substr, int64_t length)
{
    if (length < 0) {
        length = str_get_len(substr);
    }

    const char *r = str_memnstr(str->value, str->length, substr, length);
    return r != NULL;
}

bool str_starts_with_str(const Str *str, const char *prefix, int64_t length)
{
    if (length < 0) {
        length = str_get_len(prefix);
    }

    return str->length >= length && memcmp(str->value, prefix, length) == 0;
}

bool str_ends_with_str(const Str *str, const char *suffix, int64_t length)
{
    if (length < 0) {
        length = str_get_len(suffix);
    }

    return length >= 0 && memcmp(STR_TAIL_P(str) - length, suffix, length) == 0;
}

bool str_append_char(Str *str, char c)
{
    if (str_ensure_capacity(str, str->length + 2)) {
        str->value[str->length++] = c;
        str->value[str->length] = '\0';
        return true;
    }

    return false;
}

bool str_append_str(Str *str, const char *s, int64_t len)
{
    if (len < 0) {
        len = str_get_len(s);
    }

    int64_t new_length = str->length + len;
    if (str_ensure_capacity(str, new_length + 1)) {
        memcpy(STR_TAIL_P(str), s, len);
        str->value[new_length] = '\0';
        str->length = new_length;
        return true;
    }

    return false;
}

bool str_append_format(Str *str, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int length = vsnprintf(NULL, 0, format, args);
    int64_t new_length = str->length + length;

    bool result = false;

    if (str_ensure_capacity(str, new_length + 1)) {
        int written = vsnprintf(STR_TAIL_P(str), str->size - str->length, format, args);

        if (written == length) {
            result = true;
            str->value[new_length] = '\0';
            str->length = new_length;
            result = true;
        } else {
            /* Restore \0 if vsnprintf() removed it */
            str->value[str->length] = '\0';
        }
    }

    va_end(args);
    return result;
}

static char *uint_to_string(char *buffer, uint64_t n, uint64_t base)
{
    *buffer = '\0';
    do {
        *--buffer = (char) ((n % base) + '0');
        n /= base;
    } while (n > 0);

    return buffer;
}

static char *int_to_string(char *buffer, int64_t n, uint64_t base)
{
    if (n < 0) {
        char *result = uint_to_string(buffer, ~((uint64_t) n) + 1, base);
        *--result = '-';
        return result;
    } else {
        return uint_to_string(buffer, n, base);
    }
}

bool str_append_int(Str *str, int64_t value)
{
    if ((uint64_t) value <= 9) {
        return str_append_char(str, (char) (value + '0'));
    } else {
        char buffer[UINT64_MAX_STRLEN + 1];
        char *result = int_to_string(buffer + sizeof(buffer) - 1, value, 10);
        return str_append_str(str, result, (int64_t) (buffer + sizeof(buffer) - 1 - result));
    }
}

bool str_append_uint(Str *str, uint64_t value)
{
    if (value <= 9) {
        return str_append_char(str, (char) (value + '0'));
    } else {
        char buffer[UINT64_MAX_STRLEN + 1];
        char *result = uint_to_string(buffer + sizeof(buffer) - 1, value, 10);
        return str_append_str(str, result, (int64_t) (buffer + sizeof(buffer) - 1 - result));
    }
}

bool str_append_float(Str *str, double value, int precision)
{
    return str_append_format(str, "%.*f", precision, value);
}

static void str_case_convert(const Str *str, int (*convert)(int))
{
    char *s = str->value;
    const char *e = STR_TAIL_P(str);

    while (s < e) {
        *s = (char) convert(*s);
        s++;
    }
}

void str_to_lower(const Str *str)
{
    str_case_convert(str, tolower);
}

void str_to_upper(const Str *str)
{
    str_case_convert(str, toupper);
}

void str_trim(Str *str, StrTrimOptions options)
{
    const char *s = str->value;
    const char *e = STR_TAIL_P(str);

    if (options & STR_TRIM_LEFT) {
        for (; s < e; s++) {
            if (!isspace(*s)) {
                break;
            }
        }

        if (s == e) {
            /* Trim to empty string */
            str->value[0] = '\0';
            str->length = 0;
            return;
        }

        str->length -= s - str->value;
        if (s > str->value) {
            memmove(str->value, s, str->length);
        }
    }

    if (options & STR_TRIM_RIGHT) {
        for (e = e - 1; e >= s; e--) {
            if (!isspace(*e)) {
                break;
            }
        }

        str->length = e - s + 1;
    }

    str->value[str->length] = '\0';
}

bool str_repeat(Str *str, int multiply)
{
    if (multiply < 0) {
        return false;
    }

    if (multiply == 0 || str->length == 0) {
        // Truncate to empty
        str->value[0] = '\0';
        str->length = 0;
        return true;
    }

    int64_t length = str->length * multiply;
    if (str_ensure_capacity(str, length + 1)) {
        if (str->length == 1) {
            memset(str->value, str->value[0], length);
        } else {
            char *s = STR_TAIL_P(str);
            const char *e = str->value + length;

            while (s < e) {
                memmove(s, str->value, str->length);
                s += str->length;
            }
        }

        str->value[length] = '\0';
        str->length = length;
        return true;
    }

    return false;
}
