#include "utility.h"

#include <stdio.h>

#define FLASH_START 0xFF000000
#define FLASH_END 0xFF020000

#define SAVE_START 0xFF00D000
#define SAVE_END 0xFF015000

// Helpers

static bool hciReset(void) {
  const u8 buffer[4] = {0x01, 0x03, 0x0C, 0x00};
  u8 out[7] = {};
  BTData data;

  data.request = buffer;
  data.requestSize = 4;
  data.response = out;
  data.responseSize = 7;
  btTransfer(&data);
  return data.responseSize == 7;
}

static u16 hciReadRAM(u32 const address, u8 const size, u8 *out) {
  const u8 *addrBytes = (u8 const *)(&address);
  const u8 buffer[] = {
      0x01,         0x4D,         0xFC,         0x05, addrBytes[0],
      addrBytes[1], addrBytes[2], addrBytes[3], size,
  };

  BTData data;
  data.request = buffer;
  data.requestSize = 9;
  data.response = out;
  data.responseSize = 0x110;

  btTransfer(&data);
  return data.responseSize;
}

static bool hciWriteRAM(u32 const address, u8 const size, u8 const *in) {
  const u8 *addrBytes = (u8 const *)(&address);
  u8 buffer[0xFF + 0x08] = {
      0x01,         0x4C,         0xFC,         size,
      addrBytes[0], addrBytes[1], addrBytes[2], addrBytes[3],
  };
  u8 out[7] = {};

  memcpy(buffer + 0x08, in, size < 0xFF ? size : 0xFF);

  BTData data;
  data.request = buffer;
  data.requestSize = 8 + (size < 0xFF ? size : 0xFF);
  data.response = out;
  data.responseSize = 0x07;

  btTransfer(&data);
  return data.responseSize == 7;
}

static bool dumpImpl(const char *filename, u32 const from, u32 const to) {
  bool fail = false;
  FILE *h = NULL;
  u8 *out = NULL;
  u8 chunk[0x110];

  // Open file.
  h = fopen(filename, "wb+");
  if (!h) {
    printError("Could not open SD file!");
    fail = true;
    goto _dumpImpl_failure;
  }

  // Init buffers.
  out = malloc(to - from);

  if (!out) {
    printError("Could not allocate buffer!");
    fail = true;
    goto _dumpImpl_failure;
  }

  memset(out, 0x00, to - from);
  memset(chunk, 0x00, 0x110);

  // Dump.
  const u32 allSize = to - from;
  u32 index = 0;
  while ((from + index) < to) {
    const u8 dumpSize = (allSize - index) > 0xFB ? 0xFB : (allSize - index);
    iprintf("Dumping 0x%08lX - 0x%08lX\n", from + index,
            from + index + dumpSize - 1);

    if (hciReadRAM(from + index, dumpSize, chunk) < dumpSize) {
      printError("Could not read from chip!");
      fail = true;
      goto _dumpImpl_failure;
    }

    memcpy(out + index, chunk + 0x07, dumpSize);
    index += dumpSize;
  }

  // Flush content.
  fwrite(out, 1, to - from, h);

_dumpImpl_failure:
  // Free buffer.
  if (out)
    free(out);

  // Close file.
  if (h)
    fclose(h);
  return !fail;
}

static bool restoreImpl(const char *filename, u32 const from, u32 const to) {
  bool fail = false;
  FILE *h = NULL;
  u8 *in = NULL;

  // Open file.
  h = fopen(filename, "rb+");
  if (!h) {
    printError("Could not open SD file!");
    fail = true;
    goto _restoreImpl_failure;
  }

  // Init buffers.
  in = malloc(to - from);

  if (!in) {
    printError("Could not allocate buffer!");
    fail = true;
    goto _restoreImpl_failure;
  }

  memset(in, 0x00, to - from);
  fread(in, 1, to - from, h);

  // Restore.
  const u32 allSize = (to - from);
  u32 index = 0;
  while ((from + index) < to) {
    const u8 restoreSize = (allSize - index) > 0xFF ? 0xFF : (allSize - index);
    iprintf("Writing 0x%08lX - 0x%08lX\n", from + index,
            from + index + restoreSize - 1);

    if (!hciWriteRAM(from + index, restoreSize, in + index)) {
      printError("Could not write to chip!");
      fail = true;
      goto _restoreImpl_failure;
    }

    index += restoreSize;
  }

_restoreImpl_failure:
  // Free buffer.
  if (in)
    free(in);

  // Close file.
  if (h)
    fclose(h);
  return !fail;
}

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

bool dumpFlash(const char *filename) {
  return dumpImpl(filename, FLASH_START, FLASH_END);
}

bool dumpSave(const char *filename) {
  return dumpImpl(filename, SAVE_START, SAVE_END);
}

bool restoreSave(const char *filename) {
  return restoreImpl(filename, SAVE_START, SAVE_END);
}

bool testBT(void) { return hciReset(); }