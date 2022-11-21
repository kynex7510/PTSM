#include "utility.h"

#include <stdio.h>

typedef struct {
  u16 requestSize;
  const u8 *requestData;
  u16 responseSize;
  u8 *responseData;
} BTCMD, *PBTCMD;

volatile bool g_GotCmd = false;

// IRQ stuff

static void irqCmd(void) { g_GotCmd = true; }

static void waitForIrq(void) {
  do {
  } while (!g_GotCmd);
  g_GotCmd = false;
}

// SPI stuff

static void spiWait(void) {
  do {
  } while (REG_AUXSPICNT & 0x80);
}

static bool btTransfer(PBTCMD cmdData) {
  REG_ROMCTRL = 0x27416000;

  // Initialize connection.
  g_GotCmd = false;
  REG_AUXSPICNT = 0xA040;
  REG_AUXSPIDATA = 0xFF;
  spiWait();
  REG_AUXSPICNT = 0x43;
  swiDelay(20);
  waitForIrq();
  iprintf("Got init irq\n");

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

  waitForIrq();
  iprintf("Got command irq\n");

  // Send response command.
  REG_AUXSPICNT = 0xA040;
  REG_AUXSPIDATA = 0x02;
  spiWait();
  REG_AUXSPIDATA = 0x00;
  spiWait();

  // I'm 99% confident anything above is correct.
  // Here we probably need to synchronize with the cart
  // or something, idk but first two bytes (length)
  // are always 00 00, which is not good.

  // Get response size.
  for (int i = 0u; i < 5; i++) {
    swiDelay(4190000 * 5);
    REG_AUXSPIDATA = 0x00;
    spiWait();
    iprintf("BYTE 1: 0x%02X\n", REG_AUXSPIDATA);
    if (i == 4) {
      REG_AUXSPICNT = 0x00;
      REG_AUXSPIDATA = 0x00;
      spiWait();
    } else {
      REG_AUXSPIDATA = 0x00;
      spiWait();
      iprintf("BYTE 2: 0x%02X\n", REG_AUXSPIDATA);
    }
  }
  return false;
}

static bool hciReset(void) {
  const u8 buffer[4] = {0x01, 0x03, 0x0C, 0x00};
  u8 out[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  BTCMD cmdData;

  cmdData.requestSize = 4;
  cmdData.requestData = buffer;
  cmdData.responseSize = 7;
  cmdData.responseData = out;

  return btTransfer(&cmdData) ? cmdData.responseSize == 7 : false;
}

// Main

int main(void) {
  // Init screens.
  videoSetMode(MODE_0_2D);
  videoSetModeSub(MODE_0_2D);
  vramSetBankA(VRAM_A_MAIN_BG);
  consoleInit(NULL, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);

  iprintf("Setting up IRQ handlers...\n");
  irqEnable(IRQ_CARD_LINE);
  irqSet(IRQ_CARD_LINE, irqCmd);

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