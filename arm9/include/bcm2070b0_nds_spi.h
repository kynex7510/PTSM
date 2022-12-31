/**
 * bcm2070b0_nds_spi.h
 * Driver for the BCM2070B0, the bluetooth chip inside Learn With Pokémon:
 * Typing Adventure cartridges.
 */
#ifndef _BCM2070B0_NDS_SPI_H
#define _BCM2070B0_NDS_SPI_H

#include <nds.h>

typedef enum {
  BTRegion_Unknown,
  BTRegion_JPN,
  BTRegion_ITA,
  BTRegion_ENG,
  BTRegion_SPA,
  BTRegion_GER,
  BTRegion_FRA,
} BTRegion;

typedef struct {
  const u8 *request; // Input buffer.
  u16 requestSize;   // Input buffer size.
  u8 *response;      // Output buffer.
  u16 responseSize;  // Output buffer size.
} BTData;

// Get region of cartridge.
BTRegion btRegion(void);

// Transfer data.
void btTransfer(BTData *data);

#endif /* _BCM2070B0_NDS_SPI_H */