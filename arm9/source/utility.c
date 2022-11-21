#include "utility.h"

#include <stdio.h>

#define GAME_CODE_EMPTY "\x00\x00\x00\x00"

#define GAME_CODE_JPN "UZPJ"
#define GAME_TITLE_JPN "TEXASJP"

#define GAME_CODE_ITA "UZPI"
#define GAME_TITLE_ITA "TEXASIT"

#define GAME_CODE_ENG "UZPP"
#define GAME_TITLE_ENG "TEXASUK"

#define GAME_CODE_SPA "UZPS"
#define GAME_TITLE_SPA "TEXASSP"

#define GAME_CODE_GER "UZPD"
#define GAME_TITLE_GER "TEXASGE"

#define GAME_CODE_FRA "UZPF"
#define GAME_TITLE_FRA "TEXASFR"

// Globals

static u8 g_CardHeader[512];

// Helpers

static void printColor(char const *s, bool const fail) {
  if (fail)
    iprintf("\x1b[31;1m");
  else
    iprintf("\x1b[32;1m");

  iprintf("%s\x1b[39m\n", s);
}

// Adapted from nds-examples/tree/master/card/eeprom
static bool readHeader(void) {
  u8 headerCopy[512];

  // Request card access.
  enableSlot1();
  sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);

  // Read header.
  cardReadHeader(g_CardHeader);
  cardReadHeader(headerCopy);

  // Check that the header is valid.
  return !memcmp(g_CardHeader, headerCopy, 512) &&
         memcmp(g_CardHeader + 0x0C, GAME_CODE_EMPTY, 4);
}

// Utility

void printSuccess(char const *s) { printColor(s, false); }
void printError(char const *s) { printColor(s, true); }

uint32 waitForKey(void) {
  uint32 key = 0;
  while (1) {
    swiWaitForVBlank();
    scanKeys();
    key = keysDown();
    if (key)
      break;
  }

  return key;
}

TARegion getGameRegion(void) {
#define REGION_CASE(r)                                                         \
  if (!memcmp(g_CardHeader + 0x0C, GAME_CODE_##r, 4) &&                        \
      !memcmp(g_CardHeader, GAME_TITLE_##r, 7))                                \
  return TARegion_##r

  if (readHeader()) {
    REGION_CASE(JPN);
    REGION_CASE(ITA);
    REGION_CASE(ENG);
    REGION_CASE(SPA);
    REGION_CASE(GER);
    REGION_CASE(FRA);
  }

  return TARegion_Unknown;
}

char const *regionAsString(TARegion const region) {
  switch (region) {
  case TARegion_JPN:
    return "Japan";
  case TARegion_ITA:
    return "Italy";
  case TARegion_ENG:
    return "United Kingdom";
  case TARegion_SPA:
    return "Spain";
  case TARegion_GER:
    return "Germany";
  case TARegion_FRA:
    return "France";
  default:
  }

  return "(UNKNOWN)";
}