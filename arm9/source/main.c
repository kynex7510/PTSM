#include "lwptabt.h"
#include "utility.h"

#include <stdio.h>

static bool hciReset(void) {
  const u8 buffer[4] = {0x01, 0x03, 0x0C, 0x00};
  u8 out[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  BTData data;

  data.request = buffer;
  data.requestSize = 4;
  data.response = out;
  data.responseSize = 7;

  btTransfer(&data);

  iprintf("Response size: %u\n", data.responseSize);
  for (u16 i = 0; i < data.responseSize; i++)
    iprintf("Data[%u]: 0x%02X\n", i, data.response[i]);
  return data.responseSize == 7;
}

// Main

int main(void) {
  // Init screens.
  videoSetMode(MODE_0_2D);
  videoSetModeSub(MODE_0_2D);
  vramSetBankA(VRAM_A_MAIN_BG);
  consoleInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

  // Init bluetooth.
  btInit();

mainMenu:
  consoleClear();

  // Get game region.
  TARegion region = getGameRegion();
  if (region == TARegion_Unknown) {
    printError("Unknown cartridge/region!");
    iprintf("Press any key to retry...\n");
    waitForKey();
    goto mainMenu;
  }

  iprintf("Region: %s\n", regionAsString(region));

  // Get option.
  iprintf("> A: Dump savegame\n");
  iprintf("> B: Restore savegame\n");
  iprintf("> Other: Quit\n");
  // uint32 opt = waitForKeys();

  iprintf("Waiting for chip...\n");
  swiDelay(4190000 * 5); // ugly, but seems to be enough for the card to load
  iprintf("Attempting HCI reset...\n");
  if (hciReset()) {
    printSuccess("Success!");
  } else {
    printError("Failure!");
  }

  iprintf("Press any key to retry...\n");
  waitForKey();
  goto mainMenu;
  return 0;
}