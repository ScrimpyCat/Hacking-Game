## HUB Specification

This is the architecture overview for the hub computing units.

__All cycle values currently listed are for example purposes. Must test to see what works best.__

Overview
--------

Hubs are 8-bit computing devices that can talk with other hubs and modules through interface-able ports.

They consist of two operating pipelines:

* Normal - Normal execution
* Debug - Interactive execution

To operate in debugging mode, the special debug port must be used.


Ports
-----

Communicating between hubs or modules is done using ports. These ports are accessed using an ID from 0 to 255, which indicates the port used. Two hubs may have different ports connected together.

e.g. HubA may have its port 0 connected to HubB's port 55. For HubA to communicate to HubB, it will reference port 0. While for HubB to communicate to HubA, it will reference port 55.

Communication is done using two special instructions `send` (to send a message) and `recv` (to receive a message). For the communication to be successful the two communicating hubs must use the respective send/recv pair during the same timeout period. If neither gets the response during that time, the instruction will timeout and communication will have failed. It will then move onto the next instruction. If the port is not connected to anything then the instruction will fail but it will not wait the timeout period.

Port messages can be of various sizes, ranging from 0 to 255. The instructions provide no standard mechanism to know the size of the incoming data.

Data transmission is at a rate of _1/2 cycle per bit (4 cycles per byte)_. __To be tested.__


Registers
---------

Hubs have 4 general purpose registers available to them, and 2 specialty registers.

| Registers |      Type       | Encoding |                                               Description                                               |
|:---------:|:---------------:|:--------:|:-------------------------------------------------------------------------------------------------------:|
| r0        | General Purpose | 100      | Used to store both data and addresses                                                                   |
| r1        | General Purpose | 101      | Used to store both data and addresses                                                                   |
| r2        | General Purpose | 110      | Used to store both data and addresses                                                                   |
| r3        | General Purpose | 111      | Used to store both data and addresses                                                                   |
| pc        | Program Counter | 011      | Points to the instruction currently being executed                                                      |
| flags     | Status          | 010      | Contains the current state of the hub (`z` zero flag, `c` carry flag, `s` sign flag, `o` overflow flag) |

Accessing (both r/w) a register is done at a rate of _0 cycles_. __To be tested.__


### Flags:

The flags register has the bit layout `0000oscz`. The register may be used as the source or destination for any register operand. If it is used as the destination operand, the operation will modify the current value stored in the register, followed by modifying any flag bits that would normally be modified by the operation.

#### Zero Flag:

For the instructions `add`, `sub`, `mul`, `sdiv`, `udiv`, `smod`, `umod`, `cmp`, `shl`, `shr`, `xor`, `or`, `and`, `not`, `neg`, this flag is set if the result is zero. e.g. `0 + 0 = 0 (is zero)`, `0 / 1 = 0 (is zero)`, `1 / 0 = 0 (is zero)`, `1 >> 1 = 0 (is zero)`.

For the instructions `send`, `recv`, this flag is set if the port operation fails, otherwise if the port operation succeeds it is cleared.

All other instructions do not modify this flag.


#### Carry Flag:

For the instructions `add`, `sub`, `mul`, `cmp`, this flag is set if the result has carried a bit beyond the 8-bit width. e.g. `255 + 1 = 0 (carry is set)`.

For the instructions `sdiv`, `udiv`, `smod`, `umod`, this flag is set if there is a divide (mod) by zero error. e.g. `1 / 0 = 0 (carry is set)`.

For the instructions `shl`, `shr`, this flag contains the last bit carried outside the 8-bit width. e.g. `0xa0 << 1 = 0x40 (carry is set)`, `0xa0 << 2 = 0x80 (carry is cleared)`.

For the instruction `neg`, this flag is set if the source is a non-zero value, otherwise it is cleared.

For the instructions `xor`, `or`, `and`, `not`, this flag is cleared.

All other instructions do not modify this flag.


#### Sign Flag:

For the instructions `add`, `sub`, `mul`, `sdiv`, `smod`, `cmp`, `shl`, `shr`, `xor`, `or`, `and`, `not`, `neg`, this flag is set if the result has its sign bit set (most significant bit).

For the instructions `udiv`, `umod`, this flag is cleared.

All other instructions do not modify this flag.


#### Overflow Flag:

For the instructions `add`, `sub`, `mul`, `sdiv`, `smod`, `neg`, `cmp`, this flag is set if the result overflows from one sign to the other due to not enough bits. e.g. `0x7f + 1 = 0x80 (overflows from + to -)`, `0x80 - 1 = 0x7f (overflows from - to +)`, `0x80 * -1 = 0x80 (overflows from + to -)`, `-0x80 = 0x80 (overflows from + to -)`.

For the instructions `sdiv`, `udiv`, `smod`, `umod`, this flag is set if there is a divide (mod) by zero error. e.g. `1 / 0 = 0 (overflow is set)`.

For the instructions `shl`, `shr`, this flag is set if any set bits have moved outside of the 8-bit range. e.g. `0x70 << 2 = 0xc0 (overflow is set because bit originally at 0x40 has moved beyond the representable bounds)`.

For the instructions `xor`, `or`, `and`, `not`, this flag is cleared.

All other instructions do not modify this flag.


Memory
------

Each hub has 256 bytes of r/w memory (mapped from 0 to 255) available to it to use for data and instructions. Accessing (both r/w) this memory is at a rate of _1 cycle per byte_. __To be tested. Writes could potentially be faster as there's no cache?__

When executing or accessing memory that reaches beyond the bounds (255), it will wrap around.

```
send 0, 5, [0xfe] # sends 5 bytes of data from 0xfe, 0xff, 0x00, 0x01, 0x02


#or execution
#at address 0xff
nop #pc = 0xff
#after pc = 0x00
```

Instructions
------------

| Cycles | Opcode  | Mnemonic | Operand 1 | Operand 2 | Operand 3 |                                                 Description                                                  |
|:------:|:-------:|:--------:|:---------:|:---------:|:---------:|:------------------------------------------------------------------------------------------------------------:|
| 2      | 000000  | add      | r/m       | r/m       |           | Add the register or memory value (operand 2) into the register or memory (operand 1)                         |
| 2      | 000001  | add      | r/m       | i         |           | Add the immediate value (operand 2) into the register or memory (operand 1)                                  |
| 1      | 000010  | mov      | r/m       | r/m       |           | Move the register or memory value (operand 2) into the register of memory (operand 1)                        |
| 1      | 000011  | jz       | rel       |           |           | Jump if zero/equal (ZF=1)                                                                                    |
| 2      | 000100  | sub      | r/m       | r/m       |           | Subtract the register or memory value (operand 2) into the register or memory (operand 1)                    |
| 2      | 000101  | sub      | r/m       | i         |           | Subtract the immediate value (operand 2) into the register or memory (operand 1)                             |
| 1      | 000110  | mov      | r/m       | i         |           | Move the immediate value (operand 2) into the register of memory (operand 1)                                 |
| 1      | 000111  | jnz      | rel       |           |           | Jump if not zero/not equal (ZF=0)                                                                            |
| 2      | 001000  | mul      | r/m       | r/m       |           | Multiply the register or memory value (operand 2) into the register or memory (operand 1)                    |
| 2      | 001001  | mul      | r/m       | i         |           | Multiply the immediate value (operand 2) into the register or memory (operand 1)                             |
|        | 001010  |          |           |           |           |                                                                                                              |
| 1      | 001011  | js       | rel       |           |           | Jump if sign (SF=1)                                                                                          |
| 3      | 001100  | sdiv     | r/m       | r/m       |           | Signed divide by register or memory value (operand 2) into register or memory (operand 1)                    |
| 3      | 001101  | sdiv     | r/m       | i         |           | Signed divide by immediate value (operand 2) into register or memory (operand 1)                             |
|        | 001110  |          |           |           |           |                                                                                                              |
| 1      | 001111  | jns      | rel       |           |           | Jump if not sign (SF=0)                                                                                      |
| 3      | 010000  | udiv     | r/m       | r/m       |           | Unsigned divide by register or memory value (operand 2) into register or memory (operand 1)                  |
| 3      | 010001  | udiv     | r/m       | i         |           | Unsigned divide by immediate value (operand 2) into register or memory (operand 1)                           |
| 3      | 010010  |          |           |           |           |                                                                                                              |
| 1      | 010011  | jo       | rel       |           |           | Jump if overflow (OF=1)                                                                                      |
| 3      | 010100  | smod     | r/m       | r/m       |           | Signed (dividend) modulo by register or memory value (operand 2) into register or memory (operand 1)    |
| 3      | 010101  | smod     | r/m       | i         |           | Signed (dividend) modulo by immediate value (operand 2) into register or memory (operand 1)                  |
|        | 010110  |          |           |           |           |                                                                                                              |
| 1      | 010111  | jno      | rel       |           |           | Jump if not overflow (OF=0)                                                                                  |
| 3      | 011000  | umod     | r/m       | r/m       |           | Unsigned modulo by register or memory value (operand 2) into register or memory (operand 1)                  |
| 3      | 011001  | umod     | r/m       | i         |           | Unsigned modulo by immediate value (operand 2) into register or memory (operand 1)                           |
|        | 011010  |          |           |           |           |                                                                                                              |
| 4 - 12 | 011011  | send     | r         | r         | m         | Send message (operand 3) of size (operand 2) device interfacing with port (operand 1)                        |
| 2      | 011100  | cmp      | r/m       | r/m       |           | Compare the register or memory value (operand 2) with the register or memory (operand 1)                     |
| 2      | 011101  | cmp      | r/m       | i         |           | Compare the register immediate (operand 2) with the register or memory (operand 1)                           |
|        | 011110  |          |           |           |           |                                                                                                              |
| 4 - 12 | 011111  | send     | r         |           |           | Send empty message to device interfacing with port (operand 1)                                               |
| 1      | 100000  | shl      | r/m       | r/m       |           | Logical shift left by register or memory value (operand 2) into register or memory (operand 1)               |
| 1      | 100001  | shl      | r/m       | i         |           | Logical shift left by immediate value (operand 2) into register or memory (operand 1)                        |
|        | 100010  |          |           |           |           |                                                                                                              |
| 4 - 12 | 100011  | send     | i         | r         | m         | Send message (operand 3) of size (operand 2) device interfacing with port (operand 1)                        |
| 1      | 100100  | shr      | r/m       | r/m       |           | Logical shift right by register or memory value (operand 2) into register or memory (operand 1)              |
| 1      | 100101  | shr      | r/m       | i         |           | Logical shift right by immediate value (operand 2) into register or memory (operand 1)                       |
|        | 100110  |          |           |           |           |                                                                                                              |
| 4 - 12 | 100111  | recv     | r         | m         |           | Store message (operand 2) received from port (operand 1)                                                     |
| 1      | 101000  | xor      | r/m       | r/m       |           | Bitwise exclusive OR the register or memory value (operand 2) into register or memory (operand 1)            |
| 1      | 101001  | xor      | r/m       | i         |           | Bitwise exclusive OR the immediate value (operand 2) into register or memory (operand 1)                     |
|        | 101010  |          |           |           |           |                                                                                                              |
| 4 - 12 | 101011  | send     | r         | i         | m         | Send message (operand 3) of size (operand 2) device interfacing with port (operand 1)                        |
| 1      | 101100  | or       | r/m       | r/m       |           | Bitwise inclusive OR the register or memory value (operand 2) into register or memory (operand 1)            |
| 1      | 101101  | or       | r/m       | i         |           | Bitwise inclusive OR the immediate value (operand 2) into register or memory (operand 1)                     |
| 1      | 101110  | neg      | r/m       |           |           | Two's complement negation of the register or memory value (operand 1)                                        |
| 4 - 12 | 101111  | send     | i         |           |           | Send empty message to device interfacing with port (operand 1)                                               |
| 1      | 110000  | and      | r/m       | r/m       |           | Bitwise AND the register or memory value (operand 2) into register or memory (operand 1)                     |
| 1      | 110001  | and      | r/m       | i         |           | Bitwise AND the immediate value (operand 2) into register or memory (operand 1)                              |
| 1      | 110010  | not      | r/m       |           |           | One's complement negation of the register or memory value (operand 1)                                        |
| 0      | 110011  | hlt      |           |           |           | Halt the program                                                                                             |
| 1      | 110100  | jsl      | rel       |           |           | Jump if (signed) less (SF!=OF)                                                                               |
| 1      | 110101  | jsge     | rel       |           |           | Jump if (signed) greater or equal (SF=OF)                                                                    |
| 1      | 110110  | jsle     | rel       |           |           | Jump if (signed) less or equal ((ZF=1) OR (SF!=OF))                                                          |
| 1      | 110111  | jsg      | rel       |           |           | Jump if (signed) greater ((ZF=0) AND (SF=OF))                                                                |
| 1      | 111000  | jul      | rel       |           |           | Jump if (unsigned) less/carry (CF=1)                                                                         |
| 1      | 111001  | juge     | rel       |           |           | Jump if (unsigned) greater or equal/not carry (CF=0)                                                         |
| 1      | 111010  | jule     | rel       |           |           | Jump if (unsigned) less or equal (CF=1 AND ZF=1)                                                             |
| 1      | 111011  | jug      | rel       |           |           | Jump if (unsigned) greater (CF=0 AND ZF=0)                                                                   |
| 4 - 12 | 111100  | send     | i         | i         | m         | Send message (operand 3) of size (operand 2) device interfacing with port (operand 1)                        |
| 4 - 12 | 111101  | recv     | i         | m         |           | Store message (operand 2) received from port (operand 1)                                                     |
| 0      | 111110  | nop      |           |           |           | No operation                                                                                                 |
| 1      | 111111  | jmp      | rel       |           |           | Jump                                                                                                         |

The send/recv instructions can take 4 - 12 cycles because of timeout threshold. This means that if the send/recv does not pair up with another within 12 cycles then it will have reached its max threshold and continue. If the send/recv do pair up they will take a minimum of 4 cycles (if they were executed at the same time) but could take anywhere from 4 to 12. This does not include the time for the data transfer to complete.

A failed send/recv will set the z flag to one in the flags register. If they succeed they will set the z flag to zero.


### Operand Types:

| Operand |   Description    |                                                                                            Encoding                                                                                            |
|:-------:|:----------------:|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|
| i       | Immediate Value  | `iiiiiiii` - The 8-bit value                                                                                                                                                                   |
| r       | Register         | `rrr` - The register encoding                                                                                                                                                                  |
| m       | Memory Reference | `0000 iiiiiiii` - Immediate address, `0001 rr` - Register address (r0 - r3), `0010 iiiiiiii rr` - Immediate + Register address (r0 - r3), `0011 rr rr` - Register + Register address (r0 - r3) |
| rel     | Relative Address | `iiiiiiii` - The 8-bit value to use relative to the current address                                                                                                                            |

The register referencing uses only 2 bits to reference the registers r0, r1, r2, r3. It does this by using an implicit set bit `srr`, where 's' is the implicitly set bit. This does mean that the flags and pc registers are not available for addressing.

Encoding
--------

Instruction encoding takes the form of `[instruction opcode][operand 1][operand 2][operand 3]` in high-low bit order and big-endian byte order. The encoding is padded with 0 bits to fit byte boundaries.

Some example encoding arrangements:

```
[instruction][padding]
   xxxxxx       00    = 1 byte

[instruction][   i    ][padding]
   xxxxxx     iiiiiiii    00    = 2 bytes

[instruction][  rel   ][padding]
   xxxxxx     iiiiiiii    00    = 2 bytes

[instruction][ r ][padding]
   xxxxxx     rrr  0000000 = 2 bytes

[instruction][    m(i)     ][padding]
   xxxxxx     0000 iiiiiiii  000000  = 3 bytes

[instruction][  m(r) ][padding]
   xxxxxx     0001 rr   0000   = 2 bytes

[instruction][     m(i+r)     ][padding]
   xxxxxx     0010 iiiiiiii rr   0000   = 3 bytes

[instruction][  m(r+r)  ][padding]
   xxxxxx     0011 rr rr    00    = 2 bytes

[instruction][ r ][ r ][padding]
   xxxxxx     rrr  rrr   0000   = 2 bytes

[instruction][ r ][  m(r) ][padding]
   xxxxxx     rrr  0001 rr     0    = 2 bytes
```

Some example instruction encodings:

```
add r1, r0      # 00000010 11000000 : 0x02 0xc0
add flags, r0   # 00000001 01000000 : 0x01 0x40
add r1, 5       # 00000110 10000010 10000000 : 0x06 0x82 0x80
add r1, [5]     # 00000010 10000000 00101000 : 0x02 0x80 0x28
add r1, [5+r2]  # 00000010 10010000 00101100 : 0x02 0x90 0x2c
add r1, [r1+r2] # 00000010 10011011 00000000 : 0x02 0x9b 0x00

jmp 8 # 11111100 00100000 : 0xfc 0x20

nop # 11111000 : 0xf8

send r0, r2, [r3+5]  # 01101110 01100010 00000101 11000000 : 0x6e 0x62 0x05 0xc0
send 0, 0xff, [r3+5] # 11110000 00000011 11111100 10000001 01110000 : 0xf0 0x03 0xfc 0x81 0x70
```

Calculating Cycles
------------------

Cycles are calculated based on the number of cycles required to read the bytes that make up the operand + the number of cycles needed to perform the operation + any additional cycles required for any other memory read/writes (such as from memory operands).

```
mov r0, 5 # cycles: 4 = read(3) + instruction(1)
mov [r0], [r1] #cycles: 6 = read(3) + instruction(1) + src.read(1) + dest.write(1)
add [r0], r2 #cycles: 6 = read(2) + instruction(2) + dest.read(1) + dest.write(1)
```
