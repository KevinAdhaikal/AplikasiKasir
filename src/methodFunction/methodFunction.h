#ifndef METHODFUNCTION_H_INCLUDED
#define METHODFUNCTION_H_INCLUDED

#include <stdio.h>

#include "../../vendor/sandbird/sandbird.h"

void GETFunction(http_event* e);
void POSTFunction(http_event* e);
void* threadFunction(void* threadArgs);
#endif // METHODFUNCTION_H_INCLUDED