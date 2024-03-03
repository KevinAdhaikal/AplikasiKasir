#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#include "utils.h"

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

char** strsplit(const char* s, const char* delim, size_t* nb) { // https://github.com/mr21/strsplit.c
    void* data;
    char* _s = ( char* )s;
    const char** ptrs;
    size_t ptrsSize;
    size_t nbWords = 1;
    size_t sLen = strlen( s );
    size_t delimLen = strlen( delim );

    while ( ( _s = strstr( _s, delim ) ) ) {
      _s += delimLen;
      ++nbWords;
    }
    ptrsSize = ( nbWords + 1 ) * sizeof( char* );
    ptrs =
    data = malloc( ptrsSize + sLen + 1 );
    if ( data ) {
      *ptrs =
      _s = strcpy( ( ( char* )data ) + ptrsSize, s );
      if ( nbWords > 1 ) {
        while ( ( _s = strstr( _s, delim ) ) ) {
          *_s = '\0';
          _s += delimLen;
          *++ptrs = _s;
        }
      }
      *++ptrs = NULL;
    }
    if ( nb ) *nb = data ? nbWords : 0;
    return data;
}

char* formatCurrency(long long int num) {
    long long int absNum = llabs(num);
    char str[25];
    sprintf(str, "%lld", absNum);

    int len = strlen(str);
    int commaCount = (len - 1) / 3;
    int resultLength = len + commaCount + (num < 0 ? 1 : 0);
    char* resultStr = (char*)malloc((resultLength + 1) * sizeof(char));

    int i, j = 0, k = 0;
    int isNegative = (num < 0);

    if (isNegative) {
        resultStr[j++] = '-';
    }

    for (i = len - 1; i >= 0; i--) {
        resultStr[j++] = str[i];
        k++;
        if (k == 3 && i != 0) {
            resultStr[j++] = '.';
            k = 0;
        }
    }

    resultStr[resultLength] = '\0';

    for (i = isNegative ? 1 : 0; i < (resultLength + isNegative) / 2; i++) {
        char temp = resultStr[i];
        resultStr[i] = resultStr[resultLength - 1 - i + isNegative];
        resultStr[resultLength - 1 - i + isNegative] = temp;
    }

    return resultStr;
} // thanks ChatGPT

void static_string_format(char* dest, const char* message, ...) {
    va_list ap;
    va_start(ap, message);
    vsnprintf(dest, 1024, message, ap);
    va_end(ap);
}

char* dynamic_string_format(const char *format, ...) {
    va_list args;
    va_start(args, format);

    // Calculate the length of the resulting string
    int len = vsnprintf(NULL, 0, format, args) + 1;

    // Allocate memory for the resulting string
    char *result = (char *)malloc(len * sizeof(char));

    // Reinitialize args and format the string
    va_start(args, format);
    vsnprintf(result, len, format, args);
    va_end(args);

    return result;
}

char findStrLast(const char* str, const char* suffix) {
    return !strcmp(str + strlen(str) - strlen(suffix), suffix);
}

int findEmptyCRLFPosition(const char* str) {
    const char* result = strstr(str, "\r\n\r\n");
    if (result != NULL) return result - str;
    return -1;
}

int findCharNum(const char* str, char charFind) {
    int str_len = strlen(str);
    for (int a = 0; a < str_len; a++) {
        if (str[a] == charFind) return a;
    }
    return -1;
}

int findStrNum(const char* haystack, const char* needle) {
    const char* position = strstr(haystack, needle);
    
    if (position != NULL) {
        const char* extraChar = position + strlen(needle);
        if (*extraChar == '\0' || *extraChar == '.') return position - haystack;
    }

    return -1;
}

const char* MIMETypes(const char* nameFile) {
    if (findStrLast(nameFile, ".html")) return "text/html";
    else if (findStrLast(nameFile, ".txt")) return "text/plain";
    else if (findStrLast(nameFile, ".js")) return "text/javascript";
    else if (findStrLast(nameFile, ".css")) return "text/css";
    else if (findStrLast(nameFile, ".ico")) return "image/x-icon";
    else if (findStrLast(nameFile, ".woff2")) return "font/woff2";
    else return "application/octet-stream";
}

char isStr(const char* str, const char* toFind, char isEndLine) {
    int string_len = strlen(toFind);
    if (strlen(str) > string_len) return 0;

    for (int a = 0; a < string_len; a++) if (str[a] != toFind[a]) return 0;
    if (str[string_len] != '\0' && isEndLine) return 0;
    return 1;
}

char includeStr(const char* str, const char* toFind, int len) {
	int toFindLen = strlen(toFind);
	for (int a = 0, b = 0; a < len; a++) {
		if (str[a] == toFind[b]) b++;
		else b = 0;
		if (toFindLen == b) return 1;
	}
	return 0;
}

void cropString(char* dst, const char* s, int len) {
    while(len--) {
        *dst = *s;
        dst++, s++;
    }
}

int checkString(int numStrings, ...) {
    va_list args;
    va_start(args, numStrings);

    for (int a = 0; a < numStrings; a++) if(!va_arg(args, const char *)[0]) return a;

    va_end(args);
    return -1;
}

char *urlEncode(const char *str) {
    if (str == NULL) return NULL;

    size_t len = strlen(str);
    char *encodedStr = (char *)malloc(3 * len + 1); // At most, each character can become %XX, so we need 3 * len space

    if (encodedStr == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~') {
            encodedStr[j++] = c;
        } else {
            sprintf(&encodedStr[j], "%%%02X", c);
            j += 3;
        }
    }

    encodedStr[j] = '\0'; // Null-terminate the encoded string

    return encodedStr;
} // thanks ChatGPT

char is_valid_time_format(const char *str) {
    if (strlen(str) > 5) return 0;

    int hour, minute;
    if (sscanf(str, "%d:%d", &hour, &minute) != 2) return 0;

    // Memeriksa batas waktu yang valid
    if (hour < 0 || hour > 24 || minute < 0 || minute >= 60) return 0;

    return hour < 24;
} // thanks ChatGPT

int is_valid_date(char *date) {
    char extra;
    if (sscanf(date, "%*d_%*d_%*d%c", &extra) == 1) return 0;  // if there is an extra character, the format is incorrect

    int day, month, year;
    
    if (sscanf(date, "%d_%d_%d", &day, &month, &year) != 3) {
        return 0;  // if sscanf does not parse 3 values, the format is incorrect
    }

    if (year < 0) return 0;  // year cannot be negative
    if (month < 1 || month > 12) return 0;  // month has to be in the range 1-12
    if (day < 1) return 0;  // day cannot be less than 1
    if (month == 2) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) return (day <= 29);  // February in a leap year can have 29 days
        else return (day <= 28);  // February in a non-leap year can have 28 days
    } else if (month == 4 || month == 6 || month == 9 || month == 11) return (day <= 30);  // April, June, September, November have 30 days
    else return (day <= 31);  // All other months have 31 days
} // thanks ChatGPT

int convertToSeconds(char *timeStr) {
    char *ptr;
    int hour, minute, totalSeconds;
    // Mengonversi string menjadi bilangan bulat
    hour = strtol(timeStr, &ptr, 10);
    minute = strtol(ptr + 1, NULL, 10); // Menambah 1 untuk mengabaikan titik dua (:)

    // Mengonversi waktu ke detik
    totalSeconds = hour * 3600 + minute * 60;

    return totalSeconds;
} // thanks ChatGPT

int date_to_days(int year, int month) {
    struct tm time_info = {0};
    time_info.tm_year = year - 1900;
    time_info.tm_mon = month;
    time_info.tm_mday = 0;

    mktime(&time_info);

    return time_info.tm_mday;
} // thanks ChatGPT

// Function to format date as a string
char* formatDate(void* data_date) {
    struct tm* date = (struct tm*)data_date;
    char* result = (char*)malloc(12);
    snprintf(result, 12, "%d_%d_%d\n", date->tm_mday, date->tm_mon + 1, date->tm_year + 1900);
    return result;
} // thanks ChatGPT

char* dateRange(void* startDate_void, void* endDate_void) {
    struct tm startDate = *(struct tm*)startDate_void;
    struct tm endDate = *(struct tm*)endDate_void;

    time_t current = mktime(&startDate);
    time_t end = mktime(&endDate);

    char* result = (char*)malloc(1); // Allocate memory for an empty string
    result[0] = '\0'; // Initialize as an empty string

    while (current <= end) {
        struct tm *currentDate = localtime(&current);
        char* formattedDate = formatDate(currentDate);

        // Resize the result string to accommodate the new date
        result = (char*)realloc(result, strlen(result) + strlen(formattedDate) + 1);

        // Concatenate the formatted date to the result string
        strcat(result, formattedDate);

        free(formattedDate); // Free memory allocated for formattedDate

        current += 86400; // Add 1 day (in seconds)
    }

    return result;
} // thanks ChatGPT

uint32_t SuperFastHash (const char * data, int len) { // http://www.azillionmonkeys.com/qed/hash.html
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

char file_exists(const char* name_file) {
    FILE* fp = fopen(name_file, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}