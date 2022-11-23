# Protocol

Communication uses the HCI protocol over AUXSPI. 2 Commands are available.

- **{ 0x01, 0x00 }**: command used for sending HCI packets. First 2 bytes represent the size of the HCI packet, in big endian; then there is the HCI packet itself.

- **{ 0x02, 0x00 }**: command used for receving data and events. First 2 bytes, once again, represents the size of the incoming data, in big endian; next byte determines the type of the data (raw data, event data).

Before sending any command the chip must be initialized:

```c
AUXSPICNT = 0xA040;
AUXSPIDATA = 0xFF;
spiWait();
AUXSPICNT = 0x43;
delay(200);
```

An IRQ, `IRQ_CARD_LINE/IREQ_MC`, is triggered, this can be safely ignored. Once the application has sent the HCI packet, it must wait for the same IRQ to trigger again, which is the mechanism used by the chip to let the application know when the command has been handled.

As specified by the HCI protocol, the chip supports some vendor specific commands, which are reported below. All of them have OGF = 0x3F. (**TODO**: Maybe one of these is used for savegames?)

- **??? (0x6E)**: unknown