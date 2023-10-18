#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "utils.h"

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

char* formatCurrency(int num) {
    int absNum = abs(num);
    // Convert the absolute value of the integer to a string
    char str[15]; // Assuming the number will have at most 15 digits
    sprintf(str, "%d", absNum);

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
    for (int a = 0; a < strlen(str); a++) {
        if (str[a] == charFind) return a;
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

char isStr(const char* str, unsigned char* toFind, char isEndLine) {
    for (int a = 0; a < strlen(toFind); a++) {
        if (str[a] != toFind[a]) return 0;
    }
    if (str[strlen(toFind)] != '\0' && isEndLine) return 0;
    return 1;
}

char includeStr(const char* str, const unsigned char* toFind, int len) {
	int toFindLen = strlen(toFind);
	for (int a = 0, b = 0; a < len; a++) {
		if (str[a] == toFind[b]) b++;
		else b = 0;
		if (toFindLen == b) return 1;
	}
	return 0;
}

char isValidDate(int day, int month, int year) {
    if (year < 0 || month < 1 || month > 12) return 0;
    int maxDays;

    switch (month) {
        case 2:
            maxDays = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 29 : 28;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            maxDays = 30;
            break;
        default:
            maxDays = 31;
            break;
    }

    return (day >= 1 && day <= maxDays);
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