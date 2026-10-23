# CHIP-8 Emulator

A from-scratch implementation for a CHIP-8 interpreter. The CPU core has no SDL dependency (so it is unit-testable), instructions are decoded with a switch, timers run at a true 60 Hz decoupled from CPU speed, and quirk behaviour is configurable at runtime.

Reference tutorial that inspired the scope: https://austinmorlan.com/posts/chip8_emulator/

## Components:

| Component | Size | Notes |
|---|---|---|
| Memory | 4096 bytes | `0x000-0x1FF` reserved, font at `0x050`, ROM at `0x200` |
| Registers `V0-VF` | 16 x 8-bit | `VF` is the flag register (carry, borrow, collision) |
| Index `I` | 16-bit | Only low 12 bits are meaningful |
| Program counter | 16-bit | Starts at `0x200` |
| Stack | 16 x 16-bit | Return addresses |
| Stack pointer | 8-bit | Index into the stack array |
| Delay timer | 8-bit | Counts down at 60 Hz |
| Sound timer | 8-bit | Counts down at 60 Hz; buzzer on while > 0 |
| Keypad | 16 keys | `0`-`F` |
| Display | 64x32 monochrome | XOR-drawn sprites |

