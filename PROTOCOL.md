# Protocol

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
- Next bytes make up the parameters (if n = 0 a single NULL byte must end the packet).

Once the card has received the request, an IRQ is triggered... (why?).

You get the response with command `{ 0x02, 0x00 }` (FF header not sent). First two bytes is, yet again, the packet size. Next bytes are the response, which format depends on the command.

Below a list of command groups and their respective commands.

## Controller commands (OGF = 0x03)

### Reset (OCF = 0x03)

Takes no parameters. Response format is unknown.

### Write class (0CF = 0x24)

Takes 3 parameters, unknown.

### Host buffer size (OCF = 0x33)

Takes 7 parameters.

## Info commands (OGF = 0x04)

### Read local version information (OCF = 0x01)

Takes no parameters.

### Read local supported features (OCF = 0x03)

Takes no parameters.

### Read buffer size (OCF = 0x05)

Takes no parameters. Response format is unknown.

### Read BD_ADDR (OCF = 0x09)

Takes no parameters.

## Vendor specific commands (OGF = 0x3F)

### ??? (OCF = 0x6E)

Unknown.