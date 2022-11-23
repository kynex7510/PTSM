#ifndef _UTILITY_H
#define _UTILITY_H

#include "lwptabt.h"

void printSuccess(char const *s);
void printError(char const *s);
u32 waitForKey(void);
char const *regionAsString(BTRegion const region);

#endif /* _UTILITY_H */