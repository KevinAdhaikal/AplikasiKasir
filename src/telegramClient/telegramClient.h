#include <stdio.h>

int sendMessage(void* message, const char* token, const char* userid);
void* sendMessageThread(void* message);