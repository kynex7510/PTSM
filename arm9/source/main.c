#include "utility.h"

#include <stdint.h>
#include <stdio.h>

typedef struct {
  u16 requestSize;
  const u8 *requestData;
  u16 responseSize;
  u8 *responseData;
} BTCMD, *PBTCMD;

static void irqHandler(void) { iprintf("GOT IRQ!\n"); }

static void spiWait(void) {
  while (REG_AUXSPICNT & 0x80)
    ;
}

static void btTransfer(PBTCMD cmdData) {
  irqEnable(IRQ_CARD_LINE);
  irqSet(IRQ_CARD_LINE, irqHandler);

  // Initialize connection.
  REG_AUXSPICNT = 0xA040;
  REG_AUXSPIDATA = 0xFF;
  spiWait();
  REG_AUXSPICNT = 0x43;
  swiDelay(20);

  // Send request command.
  REG_AUXSPICNT = 0xA040;
  REG_AUXSPIDATA = 0x01;
  spiWait();
  REG_AUXSPIDATA = 0x00;
  spiWait();

  // Send request size.
  REG_AUXSPIDATA = (cmdData->requestSize >> 8);
  spiWait();
  REG_AUXSPIDATA = cmdData->requestSize;
  spiWait();

  // Send request data.
  for (u16 i = 0; i < cmdData->requestSize; i++) {
    if (i == (cmdData->requestSize - 1))
      REG_AUXSPICNT = 0xA000;

    REG_AUXSPIDATA = cmdData->requestData[i];
    spiWait();
  }

  // Send response command.
  REG_AUXSPICNT = 0xA040;
  REG_AUXSPIDATA = 0x02;
  spiWait();
  REG_AUXSPICNT = 0xA000;
  REG_AUXSPIDATA = 0x00;
  spiWait();

  // Get response size.
  u16 retSize = 0;
  REG_AUXSPIDATA = 0x00;
  spiWait();
  retSize = REG_AUXSPIDATA;
  REG_AUXSPIDATA = 0x00;
  spiWait();
  retSize |= ((u16)(REG_AUXSPIDATA) << 8);

  iprintf("RET SIZE: 0x%04X\n", retSize);

  cmdData->responseSize =
      (retSize < cmdData->responseSize ? retSize : cmdData->responseSize);

  // Get response data.
  for (u16 i = 0; i < cmdData->responseSize; i++) {
    if (i == (cmdData->responseSize - 1))
      REG_AUXSPICNT = 0xA000;

    REG_AUXSPIDATA = 0x00;
    spiWait();
    cmdData->responseData[i] = REG_AUXSPIDATA;
  }

  REG_AUXSPICNT = 0x00;
  spiWait();

  irqDisable(IRQ_CARD_LINE);
}

static bool hciReset(void) {
  const u8 buffer[4] = {0x01, 0x03, 0x0C, 0x00};
  BTCMD cmdData;

  cmdData.requestSize = 4;
  cmdData.requestData = buffer;
  cmdData.responseSize = -1;
  cmdData.responseData = NULL;

  btTransfer(&cmdData);
  return cmdData.responseSize == 0;
}

// Main

int main(void) {
  // Init console.
  videoSetMode(MODE_0_2D);
  vramSetBankA(VRAM_A_MAIN_BG);
  consoleInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

mainMenu:
  consoleClear();
  iprintf("--- PTSM v0.1 ---\n");

  // Read game header.
  requestCardAccess();
  tNDSHeader *header = readHeader();
  if (!header) {
    iprintf("Game not found!\n");
    iprintf("Press any key to retry...\n");
    waitForKey();
    goto mainMenu;
  }

  // Get game region.
  TARegion region = getTARegion(header);
  if (region == TARegion_Unknown) {
    iprintf("Unknown cartridge!\n");
    iprintf("Press any key to retry...\n");
    waitForKey();
    goto mainMenu;
  }

  iprintf("Game found!\n");
  iprintf("Region: %s\n", regionAsString(region));

  // Reset game.
  cardReset();

  // Get option.
  iprintf("> START: Dump savegame\n");
  iprintf("> SELECT: Restore savegame\n");
  iprintf("> OTHER: Quit\n");
  // uint32 opt = waitForKeys();

  iprintf("Attempting HCI reset...\n");
  if (hciReset()) {
    iprintf("Success!\n");
  } else {
    iprintf("Failure!\n");
  }

  iprintf("Press any key to quit...\n");
  waitForKey();
  return 0;
}