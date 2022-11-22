/*
    lwtabt.h
    Interface for the Learn With Pokémon: Typing Adventure bluetooth
    chip built in the cartridge.
*/
#ifndef _LWP_TA_BT_H
#define _LWP_TA_BT_H

#include <nds.h>

typedef struct {
  const u8 *request; // Input buffer.
  u16 requestSize;   // Input buffer size.
  u8 *response;      // Output buffer.
  u16 responseSize;  // Output buffer size.
} BTData;

void btInit(void);
void btCleanup(void);
void btTransfer(BTData *data);

#endif /* _LWP_TA_BT_H */