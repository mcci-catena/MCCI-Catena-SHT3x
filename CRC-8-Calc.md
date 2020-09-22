# CRC-8 Analysis

The SHT3x family uses a special CRC-8 polynomial to generate check codes for the input data, documented in the data sheet. Unlike many CRC algorithms performed in hardware, it simply transmits the CRC value after the checked range (rather than sending a bit-reversed complement so that the residue always ends up at a known value).

The polynomial chosen is somewhat unusual; it doesn't appear in otherwise-comprehensive lists like [Koopman et al](http://users.ece.cmu.edu/~koopman/roses/dsn04/koopman04_crc_poly_embedded.pdf).

Checking the CRC requires calculating the value on the received data. There are many possible approaches.

- It's possible to do a bitwise calculation. Since the longest message subject to CRC is only 2 bytes long, this would requires sixteen iterations through a loop.
- It's very easy to do a byte-wise calculation. An 8-bit CRC is a function of the input value and the input CRC (and in fact is a function of the XOR of the input value and the input CRC). It's easy to generate a lookup table.
- Although it was easy to generate and use a 256-byte lookup table, this seemed excessive given that the input strings to be checked are only 2 bytes long. On the other hand, I didn't like the notion of doing a bitwise CRC; it seems a waste of battery power.

I struggled for a while to create a 4-bit version of the CRC-8 algorithm. Finally, I broke down and built a gate model and did algebra. This file documents that work.

## Verilog model

Here's the model.

```verilog
// verilog for starting analysis
reg crc[7:0]

assign p7 = crc[7] ^ d[7]

always @posedge(clock)
begin
    crc[7] <= crc[6]
    crc[6] <= crc[5]
    crc[5] <= crc[4] ^ p7
    crc[4] <= crc[3] ^ p7
    crc[3[ <= crc[2]
    crc[2] <= crc[1]
    crc[1] <= crc[0]
    crc[0] <=          p7
end
```

Using this, I cranked through the algebra to evolve this into two clocks, three clocks and finally four clocks. My notation is terrible, but I use horizontal distance to represent time.

Two clocks:

```verilog
assign p7 = crc[7] ^ d[7]
assign p6 = crc[6] ^ d[6];

crc[7] <= crc[5]
crc[6] <= crc[4] ^ p7
crc[5] <= crc[3] ^ p7 ^ p6
crc[4] <= crc[2]      ^ p6
crc[3] <= crc[1]
crc[2] <= crc[0]
crc[1] <=          p7
crc[0] <=               p6
```

Three clocks:

```verilog
assign p7 = crc[7] ^ d[7];
assign p6 = crc[6] ^ d[6];
assign p5 = crc[5] ^ d[5];

crc[7] <= crc[4] ^ p7
crc[6] <= crc[3] ^ p7 ^ p6
crc[5] <= crc[2]      ^ p6 ^ p5
crc[4] <= crc[1]           ^ p5
crc[3] <= crc[0]
crc[2] <=          p7
crc[1] <-               p6
crc[0] <=                    p5
```

Four clocks:

```verilog
assign p7 = crc[7] ^ d[7];
assign p6 = crc[6] ^ d[6];
assign p5 = crc[5] ^ d[5];
assign p4 = crc[4] ^ d[4];

//                 clk1   clk2   clk3     clk4
crc[7] <= crc[3] ^  p7  ^  p6
crc[6] <= crc[2]        ^  p6  ^  p5
crc[5] <= crc[1]               ^  p5  ^ (p7 ^ p4)
crc[4] <= crc[0]                      ^ (p7 ^ p4)
crc[3] <=           p7
crc[2] <=                  p6
crc[1] <=                         p5
crc[0] <=                               (p7 ^ p4)
```

Hint: you can view the contributions of each clock left-to-right across the columns.

From this it was clear that the C++ calculation has two inputs. The first is based on the next 4 bits of the input stream, XORed with the top bits of the CRC; those bits uniquely determine the byte containing the p[x] terms, which can be grabbed from a table. The second input is bits 0..3 of the CRC, which it must be shifted left to bits 7..4, prior to combining with the bits from the table.

So we end up with this code, which can be hand-optimized further, but is enough for now.

```c++
std::uint8_t
crc8_31(
    const std::uint8_t * buf,
    size_t nBuf,
    std::uint8_t crc8
    )
    {
    static const std::uint8_t crcTable[16] =
        {
        0x00, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97,
        0xb9, 0x88, 0xdb, 0xea, 0x7d, 0x4c, 0x1f, 0x2e,
        };

    for (size_t i = nBuf; i > 0; --i, ++buf)
        {
        uint8_t b, p;

        // fetch byte, calculate first nibble
        b = *buf;
        p = (b ^ crc8) >> 4;
        crc8 = (crc8 << 4) ^ crcTable[p];

        // calculate second nibble -- this is coded for clarity.
        b <<= 4;
        p = (b ^ crc8) >> 4;
        // or in one line: p = ((crc8 >> 4) ^ b) & 0xF;

        crc8 = (crc8 << 4) ^ crcTable[p];
        }

    return crc8;
    }
```
