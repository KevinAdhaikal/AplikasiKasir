#pragma once

#include <stdbool.h>
#include <stdint.h>

#define STR_DEFAULT_INIT_SIZE 16

typedef struct Str
{
    char *value;
    int64_t size;
    int64_t length;
} Str;

typedef enum StrTrimOptions
{
    STR_TRIM_NONE = 0,
    STR_TRIM_LEFT = 1,
    STR_TRIM_RIGHT = 2,
    STR_TRIM_BOTH = 3,
} StrTrimOptions;

/**
 * Initializes a Str object of the given size.
 *
 * @param str A handle to the Str object to initialize.
 * @param size The size (in bytes) to allocate.
 *
 * @return True if the Str object was initialized; otherwise false.
 */
bool str_init_size(Str *str, int64_t size);

/**
 * Quick initializes a Str object of the given size
 *
 * @param str A handle to the Str object to initialize.
 *
 * @return True if the Str object was initialized; otherwise false.
 */
static inline bool str_init(Str *str)
{
    return str_init_size(str, STR_DEFAULT_INIT_SIZE);
}

/**
 * Finalizes the Str handle and memory resources are deallocated.
 * This function is NULL-safe. The function does nothing if NULL is passed as argument.
 *
 * @param str A handle to the Str object to finalize.
 */
void str_finalize(Str *str);

/**
 * Sets the size of the memory allocated for the Str object.
 *
 * @param str A handle to the Str object.
 * @param size The size of memory to be reallocated.
 *
 * @return True if the memory was reallocated correctly; otherwise false.
 */
bool str_set_size(Str *str, int64_t size);

/**
 * Sets the length of the string.
 *
 * @param str A handle to the Str object.
 * @param length The length to be set. If the length is less than the previous length, the string is truncated.
 * If length is greater than the previous length, the gap is filled with NULL chars. The latter may cause a
 * memory reallocation if needed.
 *
 * @return True if the length was changed correctly; otherwise false.
 */
bool str_set_length(Str *str, int64_t length);

/**
 * Tests that the allocated memory is large enough to meet the given size.
 * If it is not, then it is reallocated to twice its original size.
 *
 * @param str A handle to the Str object.
 * @param min_size The minimum required size.
 *
 * @return True if the memory is large enough or was successfully reallocated; otherwise false.
 */
bool str_ensure_capacity(Str *str, int64_t min_size);

/**
 * Copies the Str object into an uninitialized Str object.
 *
 * @param source A handle to the source Str object.
 * @param destination A handle to the uninitialized Str object. This object must not be initialized,
 * otherwise it will result in a memory leak due to double initialization.
 *
 * @return True if the copy was successful; otherwise false.
 */
bool str_copy(const Str *source, Str *destination);

/**
 * Compares the value of two Str objects.
 *
 * @param a A handle to the first Str object.
 * @param b A handle to the second Str object.
 *
 * @return -1 if the first Str object is less than the second Str object.
 * 0 if both Str objects are equal.
 * 1 if the first Str object is greater than the second Str object.
 */
int str_compare(const Str *a, const Str *b);

/**
 * Compares a Str object and a string.
 *
 * @param str A handle to the Str object.
 * @param s A pointer to the string.
 * @param length The length of the string. Pass a negative value to calculate the length internally.
 *
 * @return -1 if the Str object is less than the string.
 * 0 if the value of the Str object is equal to the string.
 * 1 if the Str object is greater than the string.
 */
int str_compare_str(const Str *str, const char *s, int64_t length);

/**
 * Returns true if the value of the Str object is equal to the given string.
 *
 * @param a A handle to the Str object.
 * @param s A pointer to the string.
 * @param length The length of the string. Pass a negative value to calculate the length internally.
 *
 * @return True if the value of the Str object is equal to the string; otherwise false.
 */
bool str_equals_str(const Str *a, const char *s, int64_t length);

/**
 * Returns the zero-based index of the first occurrence of the needle.
 *
 * @param str A handle to the Str object.
 * @param substr A pointer to the string to search.
 * @param length The length of the string. Pass a negative value to calculate the length internally.
 *
 * @return The zero-based index of the first occurrence or -1 if the needle is not present.
 */
int64_t str_indexof_str(const Str *str, const char *substr, int64_t length);

/**
 * Returns the zero-based index of the first occurrence of the needle.
 *
 * @param str A handle to the Str object.
 * @param substr A handle to the Str object to search.
 *
 * @return The zero-based index of the first occurrence or -1 if the needle is not present.
 */
static inline int64_t str_indexof(const Str *str, const Str *substr)
{
    return str_indexof_str(str, substr->value, substr->length);
}

/**
 * Returns true if both Str objects are equal.
 *
 * @param a A handle to the first Str object.
 * @param b A handle to the second Str object.
 *
 * @return True if both objects contain the same value; otherwise false.
 */
static inline bool str_equals(const Str *a, const Str *b)
{
    return str_equals_str(a, b->value, b->length);
}

/**
 * Returns true if the Str object contains the given substring.
 *
 * @param str A handle to the Str object.
 * @param substr A pointer to the string to search.
 * @param length The length of the string. Pass a negative value to calculate the length internally.
 *
 * @return True if the Str object contains the given substring; otherwise false.
 */
bool str_contains_str(const Str *str, const char *substr, int64_t length);

/**
 * Returns true if the Str object contains the given substring.
 *
 * @param str A handle to the Str object.
 * @param substr A handle to the Str object to search.
 *
 * @return True if the Str object contains the given substring; otherwise false.
 */
static inline bool str_contains(const Str *str, const Str *substr)
{
    return str_contains_str(str, substr->value, substr->length);
}

/**
 * Returns true if the value of the Str object starts with the given prefix.
 *
 * @param str A handle to the Str object.
 * @param prefix A pointer to the string.
 * @param length The length of the prefix. Pass a negative value to calculate the length internally.
 *
 * @return True if the Str object starts with the prefix.
 */
bool str_starts_with_str(const Str *str, const char *prefix, int64_t length);

/**
 * Returns true if the value of the Str object starts with the given prefix.
 *
 * @param str A handle to the Str object.
 * @param prefix A handle to the Str object to use as prefix.
 *
 * @return True if the Str object starts with the prefix.
 */
static inline bool str_starts_with(const Str *str, const Str *prefix)
{
    return str_starts_with_str(str, prefix->value, prefix->length);
}

/**
 * Returns true if the value of the Str object ends with the given suffix.
 *
 * @param str A handle to the Str object.
 * @param suffix A pointer to the string.
 * @param length The length of the suffix. Pass a negative value to calculate the length internally.
 *
 * @return True if the Str object ends with the suffix.
 */
bool str_ends_with_str(const Str *str, const char *suffix, int64_t length);

/**
 * Returns true if the value of the Str object ends with the given suffix.
 *
 * @param str A handle to the Str object.
 * @param suffix A pointer to the string.
 * @param length The length of the suffix. Pass a negative value to calculate the length internally.
 *
 * @return True if the Str object ends with the suffix.
 */
static inline bool str_ends_with(const Str *str, const Str *suffix)
{
    return str_ends_with_str(str, suffix->value, suffix->length);
}

/**
 * Appends a character.
 *
 * @param str A handle to the Str object.
 * @param c The character to append.
 *
 * @return True if the character was appended successfully; otherwise false.
 */
bool str_append_char(Str *str, char c);

/**
 * Appends a string.
 *
 * @param str A handle to the Str object.
 * @param s A pointer to the string to append.
 * @param len The length of the string. Pass a negative value to calculate the length internally.
 *
 * @return True if the string was appended successfully; otherwise false.
 */
bool str_append_str(Str *str, const char *s, int64_t len);

/**
 * Appends a formatted string. (Like printf).
 *
 * @param str A handle to the Str object.
 * @param format The format string.
 * @param ... A vararg list of arguments for the format string.
 *
 * @return True if the formatted string was appended successfully; otherwise false.
 */
bool str_append_format(Str *str, const char *format, ...);

/**
 * Appends a signed 64-bit integer.
 *
 * @param str A handle to the Str object.
 * @param value The value to append.
 *
 * @return True if the integer was appended successfully; otherwise false.
 */
bool str_append_int(Str *str, int64_t value);

/**
 * Appends an unsigned 64-bit integer.
 *
 * @param str A handle to the Str object.
 * @param value The value to append.
 *
 * @return True if the integer was appended successfully; otherwise false.
 */
bool str_append_uint(Str *str, uint64_t value);

/**
 * Appends a double precision floating point (AKA double) value.
 *
 * @param str A handle to the Str object.
 * @param value The value to append.
 * @param precision The number of decimals to insert.
 *
 * @return True if the value was appended successfully; otherwise false.
 */
bool str_append_float(Str *str, double value, int precision);

/**
 * Concatenates the value of another Str object.
 *
 * @param str A handle to the resulting Str object.
 * @param other A handle to the Str object to concatenate.
 *
 * @return True if the concatenation was successful; otherwise false.
 */
static inline bool str_concat(Str *str, const Str *other)
{
    return str_append_str(str, other->value, other->length);
}

/**
 * Converts the value of the Str object to uppercase.
 *
 * @param str A handle to the Str object.
 */
void str_to_lower(const Str *str);

/**
 * Converts the value of the Str object to lowercase.
 *
 * @param str A handle to the Str object.
 */
void str_to_upper(const Str *str);

/**
 * Strips whitespace from the beginning and the end of the Str object.
 *
 * @param str A handle to the Str object.
 * @param options Trim options.
 */
void str_trim(Str *str, StrTrimOptions options);

/**
 * Repeats the string n times.
 *
 * @param str A handle to the Str object.
 * @param multiply The number of times to repeat the string. A value of 0 will truncate the string to empty.
 * If the value is negative, the string remains unchanged and returns false.
 *
 * @return True if the string was repeated; otherwise false.
 */
bool str_repeat(Str *str, int multiply);
