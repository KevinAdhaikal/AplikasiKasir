#include <stdio.h>
#include <stdlib.h>

#include "funcVarPub.h"

telegramBot teleBot;
tele_thread_info teleThreadInfo;
#ifdef _WIN32
HANDLE tele_thread[1];
#else
pthread_t tele_thread[1];
#endif