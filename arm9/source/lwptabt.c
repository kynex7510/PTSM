#include "lwptabt.h"

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

volatile bool g_HasTriggeredIRQ = false;

// Helpers

static BTRegion getGameRegion(u8 const *header) {
#define REGION_CASE(r)                                                         \
  if (!memcmp(header + 0x0C, GAME_CODE_##r, 4) &&                              \
      !memcmp(header, GAME_TITLE_##r, 7))                                      \
  return BTRegion_##r

  REGION_CASE(JPN);
  REGION_CASE(ITA);
  REGION_CASE(ENG);
  REGION_CASE(SPA);
  REGION_CASE(GER);
  REGION_CASE(FRA);

  return BTRegion_Unknown;
}

static void handleIRQ(void) { g_HasTriggeredIRQ = true; }
static void resetChipState(void) { g_HasTriggeredIRQ = false; }

static void waitForChip(void) {
  do {
  } while (!g_HasTriggeredIRQ);
}

static void spiSet(u16 const cnt) { REG_AUXSPICNT = cnt; }

static void spiWait(void) {
  do {
  } while (REG_AUXSPICNT & 0x80);
}

static u8 spiTransfer(u8 const b) {
  REG_AUXSPIDATA = b;
  spiWait();
  return REG_AUXSPIDATA;
}

// LWPTABT

BTRegion btInit(void) {
  u8 gameHeader[0x200];
  u8 headerCopy[0x200];

  // Request card access.
  enableSlot1();
  sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);

  // Initialize cartridge.
  cardReadHeader(gameHeader);
  cardReadHeader(headerCopy);

  // Validate the header.
  if (memcmp(gameHeader, headerCopy, 0x200) ||
      !memcmp(gameHeader + 0x0C, GAME_CODE_EMPTY, 4))
    return BTRegion_Unknown;

  // Get region.
  BTRegion region = getGameRegion(gameHeader);
  if (region != BTRegion_Unknown) {
    // Setup IRQ.
    irqSet(IRQ_CARD_LINE, handleIRQ);
    irqEnable(IRQ_CARD_LINE);

    // TODO: sync with chip?
    swiDelay(4190000 * 5); // Ugly, but seems to be enough for the chip to load.
  }

  return region;
}

void btCleanup(void) {
  irqClear(IRQ_CARD_LINE);
  disableSlot1();
}

void btTransfer(BTData *data) {
  // Check parameters.
  if (!data)
    return;

  const u16 invalidSize = data->responseSize ? 0 : 0xFFFF;
  if (!data->request || !data->requestSize) {
    data->responseSize = invalidSize;
    return;
  }

  // Build commands.
  const u8 reqCmd[4] = {0x01, 0x00, data->requestSize >> 8, data->requestSize};
  const u8 resCmd[2] = {0x02, 0x00};

  // Initialize connection.
  const u16 oldCnt = REG_AUXSPICNT;
  spiSet(0xA040);
  spiTransfer(0xFF);
  spiSet(0x43);
  swiDelay(200);

  // Send request.
  spiSet(0xA040);
  for (int i = 0; i < 4; i++)
    spiTransfer(reqCmd[i]);

  for (u16 i = 0; i < data->requestSize; i++) {
    if (i == (data->requestSize - 1)) {
      resetChipState();
      spiSet(0xA000);
    }

    spiTransfer(data->request[i]);
  }

  // Wait for command to be processed.
  waitForChip();

  // Get response.
  spiSet(0xA040);
  for (int i = 0; i < 2; i++)
    spiTransfer(resCmd[i]);

  u16 resSize = (u16)(spiTransfer(0x00)) << 8;
  resSize |= spiTransfer(0x00);
  data->responseSize = resSize <= data->responseSize ? resSize : invalidSize;

  if (data->responseSize != invalidSize) {
    for (u16 i = 0; i < data->responseSize; i++) {
      if (i == (data->responseSize - 1))
        spiSet(0xA000);

      const u8 b = spiTransfer(0x00);
      if (data->response)
        data->response[i] = b;
    }
  }

  REG_AUXSPICNT = oldCnt;
}