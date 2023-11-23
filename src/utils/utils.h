#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

char** strsplit(const char* s, const char* delim, size_t* nb);
void static_string_format(char* dest, const char *message, ...);
char* dynamic_string_format(const char *format, ...);
char findStrLast(const char* str, const char* strFind);
int findEmptyCRLFPosition(const char* str);
int findCharNum(const char* str, char charFind);
const char* MIMETypes(const char* nameFile);
char isStr(const char* str, unsigned char* toFind, char isEndLine);
char includeStr(const char* str, const unsigned char* toFind, int len);
int findEmptyNewlinePosition(const char* str);
void cropString(char* dst, const char* s, int len);
int checkString(int numStrings, ...);
char* formatCurrency(int num); // thanks ChatGPT
char *urlEncode(const char *str); // thanks ChatGPT
int is_valid_date(char *date); // thanks ChatGPT
char is_valid_time_format(const char* str); // thanks ChatGPT
int convertToSeconds(char *timeStr); // thanks ChatGPT
int date_to_days(int year, int month); // thanks ChatGPT
char* formatDate(void* data_date); // thanks ChatGPT
char* dateRange(void* startDate_void, void* endDate_void); // thanks ChatGPT

#endif // UTILS_H_INCLUDED
