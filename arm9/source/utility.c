#include "utility.h"

#include <stdio.h>

// Helpers

static void printColor(char const *s, bool const fail) {
  if (fail)
    iprintf("\x1b[31;1m");
  else
    iprintf("\x1b[32;1m");

  iprintf("%s\x1b[39m\n", s);
}

// Utility

void printSuccess(char const *s) { printColor(s, false); }
void printError(char const *s) { printColor(s, true); }

u32 waitForKey(void) {
  u32 key = 0;
  while (1) {
    swiWaitForVBlank();
    scanKeys();
    key = keysDown();
    if (key)
      break;
  }

  return key;
}

char const *regionAsString(BTRegion const region) {
  switch (region) {
  case BTRegion_JPN:
    return "Japan";
  case BTRegion_ITA:
    return "Italy";
  case BTRegion_ENG:
    return "United Kingdom";
  case BTRegion_SPA:
    return "Spain";
  case BTRegion_GER:
    return "Germany";
  case BTRegion_FRA:
    return "France";
  default:
  }

  return "(UNKNOWN)";
}