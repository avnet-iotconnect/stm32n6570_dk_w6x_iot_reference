# Flash and RAM Memory Layout

This document describes the memory organization and boot sequence for the STM32N6570-DK with FSBL (First Stage BootLoader) and application firmware.

## Overview

The STM32N6570 uses a **Load and Run** architecture where the FSBL loads the application from external flash into RAM before execution. This enables flexible code execution from a large external flash storage while maintaining performance through RAM execution.

For technical details on this boot model, see the ST community guide: [How to create an STM32N6 FSBL Load and Run](https://community.st.com/t5/stm32-mcus/how-to-create-an-stm32n6-fsbl-load-and-run/ta-p/768206)

## Flash Layout (External Flash - MX66LM1G45G)

**Base Address**: `0x70000000`

| Region | Address | Size | Purpose |
|--------|---------|------|---------|
| **FSBL** | `0x70000000` | 1 MB (16 blocks) | First Stage BootLoader image |
| **Application** | `0x70100000` | ~1 MB | Application firmware image (copied to RAM by FSBL) |
| **LittleFS** | `0x70400000`¹ | Remaining | File system for PKCS#11/KVS storage |

¹ **LFS Start Address Calculation**:
- `XPI_START_ADDRESS = MX66LM_RESERVED_BLOCKS × MX66LM_BLOCK_SZ`
- `XPI_START_ADDRESS = 64 × 65536 = 0x400000`
- `LFS Base = 0x70000000 + 0x400000 = 0x70400000`

**Total External Flash**: 512 Mbit (64 MB) organized as:
- 1024 blocks of 64 KB each
- 16 sectors (4 KB) per block
- First 64 blocks (4 MB) reserved for FSBL and Appli images

## RAM Layout (Internal STM32N6570)

| Address | Size | Purpose |
|---------|------|---------|
| `0x34000000` | 0x400 bytes | Application header (metadata, CRC, etc.) |
| `0x34000400` | ~1 MB | Application code and initialized data (copied from external flash by FSBL) |
| `0x34100000` | Remaining available | Application runtime RAM (variables, stack, heap) |
| `0x34180400` | FSBL runtime | FSBL location during boot (allocated by ROM bootloader) |

**Notes**:
- Application RAM at `0x34100000` can be extended to use all available STM32N6 internal RAM
- Total available internal RAM is determined by the specific STM32N6570 variant
- FSBL runtime location (`0x34180400`) becomes available to the application after boot completes

## Boot Sequence

1. **ROM Boot** → Internal STM32N6570 ROM bootloader executes
2. **Load FSBL** → ROM bootloader loads FSBL from external flash (`0x70000000`) to internal RAM at `0x34180400`
3. **Jump to FSBL** → ROM bootloader jumps to FSBL at `0x34180400`
4. **FSBL Copies Application** → FSBL reads application image from external flash (`0x70100000`) and copies it to internal RAM starting at `0x34000000`
5. **Jump to Application** → FSBL jumps to application entry point at `0x34000400`
6. **Application Executes** → Application runs from internal RAM with access to:
   - Code/initialized data at `0x34000000` - `0x34100000`
   - Runtime RAM (stack, heap, variables) at `0x34100000` onwards
   - FSBL memory at `0x34180400` now available for application use

## Application Addresses

**External Flash (Storage)**:
- **FSBL Location**: `0x70000000` (flashed from build output)
- **Application Location**: `0x70100000` (flashed from build output)
  - Entire application image stored here, including header
  - FSBL reads from this location during boot

**Internal RAM (Execution)**:
- **Application Header**: `0x34000000` - `0x34000400`
  - First `0x400` bytes contain metadata (CRC, version, etc.)

- **Application Code/Data**: `0x34000400` onwards
  - FSBL copies entire image (including header) to `0x34000000`
  - Entry point jumps to `0x34000400` where actual code begins

- **Application Runtime RAM**: `0x34100000` - end of available internal RAM
  - Stack, heap, and global variables
  - Can extend to utilize all available STM32N6 internal RAM

## Linker Script Configuration

The application linker script defines these memory regions:

**File**: `Appli/STM32N657X0HXQ_LRUN.ld`

```linker
MEMORY
{
  ROM    (xrw)    : ORIGIN = 0x34000400,   LENGTH = 1023K
  RAM    (xrw)    : ORIGIN = 0x34100000,   LENGTH = 1024K
}
```

- **ROM**: Specifies where the application image is located in system RAM
- **RAM**: Specifies where the application executes in system RAM

The linker script ensures:
- Code and initialized data sections are placed in ROM (will be copied to RAM by FSBL)
- Runtime sections (stack, heap, uninitialized data) are placed in RAM

## Storage Backend (LittleFS)

PKCS#11 and KVS use external flash starting at `0x70400000` (after reserved blocks) for:
- Device certificates and keys
- Runtime configuration (Wi-Fi credentials, MQTT endpoint, etc.)
- Persistent application data

**Calculation**:
```
LFS_BASE = 0x70000000 (external flash base)
         + 0x400000 (64 reserved blocks × 64 KB)
         = 0x70400000
```

See [docs/software_components.md](software_components.md) for details on PKCS#11/KVS architecture.
