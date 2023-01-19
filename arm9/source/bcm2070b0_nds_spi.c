#include "bcm2070b0_nds_spi.h"

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
static void resetIRQState(void) { g_HasTriggeredIRQ = false; }

static void waitForIRQ(void) {
  do {
  } while (!g_HasTriggeredIRQ);
}

static u16 spiGet(void) { return REG_AUXSPICNT; }
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

// BCM2070B0 NDS SPI

BTRegion btRegion(void) {
  u8 gameHeader[0x200];
  u8 headerCopy[0x200];

  irqSet(IRQ_CARD_LINE, handleIRQ);
  irqEnable(IRQ_CARD_LINE);

  // Read card header.
  sysSetCardOwner(BUS_OWNER_ARM9);
  cardReadHeader(gameHeader);
  cardReadHeader(headerCopy);

  // Validate the header.
  if (memcmp(gameHeader, headerCopy, 0x200) ||
      !memcmp(gameHeader + 0x0C, GAME_CODE_EMPTY, 4))
    return BTRegion_Unknown;

  // Get region.
  return getGameRegion(gameHeader);
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
  const u16 oldCnt = spiGet();
  spiSet(0xA040);
  spiTransfer(0xFF);
  spiSet(0x43);
  resetIRQState();

  // Send request.
  spiSet(0xA040);
  waitForIRQ();

  for (int i = 0; i < 4; i++)
    spiTransfer(reqCmd[i]);

  for (u16 i = 0; i < data->requestSize; i++) {
    if (i == (data->requestSize - 1)) {
      resetIRQState();
      spiSet(0xA000);
    }

    spiTransfer(data->request[i]);
  }

  // Wait for command to be processed.
  waitForIRQ();

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

  // Restore old status.
  spiSet(oldCnt);
}