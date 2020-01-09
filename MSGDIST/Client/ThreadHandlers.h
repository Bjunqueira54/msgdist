#ifndef THREADHANDLERS_H
#define THREADHANDLERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "clientHeader.h"

void* receiveTopicList(void*);
void* UserInputThreadHandler(void*);

#ifdef __cplusplus
}
#endif

#endif