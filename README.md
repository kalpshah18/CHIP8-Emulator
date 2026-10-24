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


## Requirements

- CMake 3.16+
- A C++17 compiler (MSVC, GCC or Clang)
- SDL2 (installed with [vcpkg](https://github.com/microsoft/vcpkg): `vcpkg install sdl2:x64-windows`)

## Building

The core library has no SDL dependency; only the `chip8` executable links against SDL2. Point CMake at the vcpkg toolchain file when configuring:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

On Linux/macOS, install SDL2 with your package manager and omit the toolchain flag. The executable is written to `build/Release/chip8.exe` (Visual Studio generators) or `build/chip8` (single-config generators).

If Windows reports that `SDL2.dll` is missing, copy it from `<vcpkg>/installed/x64-windows/bin/` next to the executable.

## Usage

```powershell
.\build\Release\chip8.exe <rom> [--scale N] [--ips N] [quirk flags]
```

| Option | Default | Description |
|---|---|---|
| `--scale N` | 12 | Window pixels per CHIP-8 pixel |
| `--ips N` | 700 | Instructions per second (minimum 60) |
| `--shift-vy` | off | `8XY6`/`8XYE` shift `VY` instead of `VX` |
| `--inc-i` | off | `FX55`/`FX65` increment `I` |
| `--reset-vf` | off | `8XY1`/`8XY2`/`8XY3` reset `VF` |
| `--jump-vx` | off | `BNNN` jumps to `NNN + VX` instead of `NNN + V0` |

Example:

```powershell
.\build\Release\chip8.exe roms\space-invaders\Space-Invaders.ch8 --scale 12 --ips 700
```

Press **Esc** to quit. If a ROM misbehaves, try different quirk flags or a different `--ips`.

## Controls

The hex keypad is mapped to the left side of a QWERTY keyboard:

```
CHIP-8 keypad      Keyboard
 1 2 3 C            1 2 3 4
 4 5 6 D            Q W E R
 7 8 9 E            A S D F
 A 0 B F            Z X C V
```

For Space Invaders: **W** starts the game and fires, **Q** moves left, **E** moves right.

## Testing

Run the [chip8-test-rom](roms/chip8-test-rom) `test_opcode.ch8` ROM to check opcode behaviour; each opcode shows `OK` or `NO`:

```powershell
.\build\Release\chip8.exe roms\chip8-test-rom\test_opcode.ch8
```

## License

Released under the [MIT License](LICENSE).
