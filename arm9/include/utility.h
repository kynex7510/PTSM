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

uint32 waitForKey(void);
void requestCardAccess(void);
tNDSHeader *readHeader(void);
TARegion getTARegion(tNDSHeader const *header);
const char *regionAsString(TARegion const region);

#endif /* _UTILITY_H */