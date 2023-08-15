#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

char** strsplit(const char* s, const char* delim, size_t* nb);
void CatchMessage(char* dest, const char *message, ...);
char findStrLast(const char* str, const char* strFind);
int findEmptyCRLFPosition(const char* str);
int findCharNum(const char* str, char charFind);
const char* MIMETypes(const char* nameFile);
char isStr(const char* str, unsigned char* toFind, char isEndLine);
char includeStr(const char* str, const unsigned char* toFind, int len);
int findEmptyNewlinePosition(const char* str);
char isValidDate(int day, int month, int year);
void cropString(char* dst, const char* s, int len);
int checkString(int numStrings, ...);
char* formatCurrency(int num); // thanks ChatGPT
char *urlEncode(const char *str); // thanks ChatGPT
#endif // UTILS_H_INCLUDED
