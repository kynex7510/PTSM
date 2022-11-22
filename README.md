# PTSM

RE notes about the bluetooth chip inside Learn With Pokémon: Typing Adventure. Hopefully soon to become a savedata manager.

## Mysteries

- Commands get ducked if we dont have a call to swiDelay before. An arbitrary value of `4190000 * 5` seems to work.
- - Is there a built-in mechanism that lets the game know when the chip is ready?
- - If yes, why does it work (get to error screen) on emulators instead of hanging?
- - If no, does the game assume the card is initialized?
- - Is any of the other IRQs involved?

- Removing any of the iprintf calls in `btTransfer` make the chip return trash data as response. Replacing any iprintf call with swiDelay, even with big values, does not make it work either. Does iprintf do something magical under the hood?

- Response data always misses the size field. It's always set to 00 00. No amount of swiDelay nor iprintf seems to affect this. Other parts of the packet seem fine.

- Sending the same command twice leads to a trash response second time.