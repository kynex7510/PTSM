#include "bcm2070b0_nds_spi.h"
#include "utility.h"

#include <fat.h>
#include <stdio.h>
#include <stdlib.h>

#define FLASH_FILE "sd:/PTSM_Flash_Dump.bin"
#define SAVE_FILE "sd:/PTSM_Save_Dump.bin"

// Main

int main(void) {
  // Init screens.
  videoSetMode(MODE_0_2D);
  videoSetModeSub(MODE_0_2D);
  vramSetBankA(VRAM_A_MAIN_BG);
  consoleInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

  // Init libfat.
  if (!fatInitDefault()) {
    printError("FAT init failed!");
    iprintf("Press any key to quit...\n");
    waitForKey();
    return 0;
  }

  // Enable card access.
  enableSlot1();

  // Main loop.
  bool quit = false;
  while (true) {
    consoleClear();

    // Get region.
    BTRegion region = btRegion();
    while (region == BTRegion_Unknown) {
      printError("Unknown cartridge/region!");
      iprintf("Press any key to retry...\n");
      waitForKey();
      consoleClear();
      region = btRegion();
    }

    swiDelay(0x82EA * 200); // 200ms

    iprintf("Region: %s\n", regionAsString(region));
    iprintf("> A: Dump savegame\n");
    iprintf("> B: Restore savegame\n");
    iprintf("> X: Dump flash\n");
    iprintf("> Y: Test bluetooth chip\n");
    iprintf("> Other: Quit\n");

    u32 opt = waitForKey();

    if (opt & KEY_A) {
      if (dumpSave(SAVE_FILE))
        printSuccess("Savegame dumped successfully!");
    } else if (opt & KEY_B) {
      if (restoreSave(SAVE_FILE))
        printSuccess("Savegame restored successfully!");
    } else if (opt & KEY_X) {
      if (dumpFlash(FLASH_FILE))
        printSuccess("Flash dumped successfully!");
    } else if (opt & KEY_Y) {
      if (testBT())
        printSuccess("Bluetooth test succeeded!");
      else
        printError("Bluetooth test failed!");
    } else {
      quit = true;
    }

    if (quit)
      break;

    iprintf("Press any key to continue...\n");
    waitForKey();
  }

  return 0;
}