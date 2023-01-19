#ifndef _UTILITY_H
#define _UTILITY_H

#include "bcm2070b0_nds_spi.h"

void printSuccess(char const *s);
void printError(char const *s);
u32 waitForKey(void);
char const *regionAsString(BTRegion const region);
bool dumpFlash(const char *filename);
bool dumpSave(const char *filename);
bool restoreSave(const char *filename);
bool testBT(void);

#endif /* _UTILITY_H */