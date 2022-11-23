# PTSM

RE notes about the bluetooth chip inside Learn With Pokémon: Typing Adventure. Hopefully soon to become a savedata manager.

## Mysteries

IRQ is not sent if we dont do `swiDelay(4190000 * 5)` at the start. Moreover, commands send successfully only if sequential, ie. in the case of our loop, only the first time will work. Interestingly, values other than `4190000 * 5`, even greater, wont work and will hang waiting for the IRQ. This suggests that the code somehow has to synchronize with the cartridge. The question then becomes: how?