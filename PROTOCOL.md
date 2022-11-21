# Protocol

Before doing anything the card must be initialized? A delay seems to do the trick, but this game is weird af.

Communication uses SPI. A transaction is defined by a request and a response. Every transaction starts with `SPICNT = 0xA040` and `SPIDATA = { 0xFF }`; after that, `SPICNT = 0x43` for whatever reason. Every request is sent using `SPIDATA = { 0x01, 0x00 }` with `SPICNT = 0xA040`. First two bytes after that are the packet size in big endian, next bytes are HCI packets. HCI packets are structured as so:
- First byte is packet type. Possible values:
```
0x01 = command
0x02 = async data
0x03 = sync data (should be unused)
0x04 = event
0x09 = extension/custom command
```
- Next two bytes are the "Opcode Group Field" and "Opcode Command Field". Bytes must be transformed from big endian to little endian, then OGF is `x >> 10`, while OCF is `x & 0x3FF`.

- Next byte is number of parameters (max 0xFF - 255).
- Next bytes make up the parameters (or none if n = 0).

You get the response with command `{ 0x02, 0x00 }`. First two bytes is, yet again, the packet size. Next bytes are the response, which format depends on the command.

Once the card has received the request, an IRQ (IREQ_MC) is triggered. This is simply a synchronization mechanism, as the code that sends the commands and the one that receives the output run on separate threads. The IRQ handler sends a message to each thread, depending on what the last operation was (if a command is sent, the reader thread is woken up; if a read happened, the sender thread gets ready). An IRQ is also triggered when sending the header byte `0xFF`, but this is ignored by the game.

Transaction example:

```
First we write to the SPI bus:

FF      // starts transaction

/* IREQ_MC triggered: card is listening for a command */

01 00 	// prepare chip to receive command
00 04 	// size of command payload
01  	// HCI packet type ("command" in this case)
03 0C 	// HCI OGF and OCF (OGF = controller, OCF = reset)
00      // num of params

/* IREQ_MC triggered: card successfully received the command */

02 00 	// Ask chip for data

From now on, we read from the bus:

YY XX 	// Response size
.....	// Response data

/* IREQ_MC triggered: card is ready for another command */
```

**List of vendor specific HCI commands (OGF = 0x3F)**

Maybe one of these is used for savegames

### ??? (OCF = 0x6E)

Unknown.

todo...