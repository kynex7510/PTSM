#include "lwptabt.h"

// Globals

volatile bool g_HasTriggeredIRQ = false;

// Helpers

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

void btInit(void) {
  irqSet(IRQ_CARD_LINE, handleIRQ);
  irqEnable(IRQ_CARD_LINE);
}

void btCleanup(void) {
  irqDisable(IRQ_CARD_LINE);
  irqClear(IRQ_CARD_LINE);
}

void btTransfer(BTData *data) {
  const u8 reqCmd[4] = {0x01, 0x00, data->requestSize >> 8, data->requestSize};
  const u8 resCmd[2] = {0x02, 0x00};

  // Initialize connection.
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

  data->responseSize = resSize <= data->responseSize ? resSize : 0;
  for (u16 i = 0; i < data->responseSize; i++) {
    if (i == (data->responseSize - 1))
      spiSet(0xA000);

    data->response[i] = spiTransfer(0x00);
  }

  // TODO: how to gracefully terminate the connession?
  REG_AUXSPICNT = 0x00;
  REG_AUXSPIDATA = 0x00;
}