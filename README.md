# PTSM

RE notes about the bluetooth chip inside Learn With Pokémon: Typing Adventure. Hopefully soon to become a savedata manager.

## Mysteries

- Commands get ducked if we dont have a call to swiDelay before. An arbitrary value of `4190000 * 5` seems to work.
- - Is there a built-in mechanism that lets the game know when the chip is ready?
- - If yes, why does it work (get to error screen) on emulators instead of hanging?
- - If no, does the game assume the card is initialized?
- - Is any of the other IRQs involved?

- Sending the same command twice leads to a trash response second time/IRQ not triggered. We probably need to terminate the connection in some way.