#ifndef _UTILITY_H
#define _UTILITY_H

#include <nds.h>

typedef enum {
  TARegion_Unknown,
  TARegion_JPN,
  TARegion_ITA,
  TARegion_ENG,
  TARegion_SPA,
  TARegion_GER,
  TARegion_FRA,
} TARegion;

void printSuccess(char const *s);
void printError(char const *s);
uint32 waitForKey(void);
TARegion getGameRegion(void);
char const *regionAsString(TARegion const region);

#endif /* _UTILITY_H */