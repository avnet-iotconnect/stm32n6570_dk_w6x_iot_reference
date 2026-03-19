# STM32N6570-DK IoT Reference (ST67W611M1, FreeRTOS, MQTT over TLS)

[![Board: STM32N6570-DK](https://img.shields.io/badge/Board-STM32N6570--DK-03234B)](https://www.st.com/en/evaluation-tools/stm32n6570-dk.html)
[![RTOS: FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-1A73E8)](https://www.freertos.org/)
[![Network: LwIP](https://img.shields.io/badge/Network-LwIP-006064)](https://savannah.nongnu.org/projects/lwip/)
[![TLS: MbedTLS 3.1.1](https://img.shields.io/badge/TLS-MbedTLS%203.1.1-283593)](https://www.keil.arm.com/packs/mbedtls-arm/versions/)
[![Wi-Fi: ST67W611M1](https://img.shields.io/badge/Wi--Fi-ST67W611M1-0B8043)](https://www.st.com/content/st_com/en/campaigns/st67w-wifi6-bluetooth-thread-module-z13.html)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](./LICENSE.md)

This repository is a practical IoT firmware reference for **STM32N6570-DK** using the **ST67W611M1** Wi-Fi module. It demonstrates **MQTT over TLS** with a modular software stack based on **FreeRTOS**, **LwIP**, and **MbedTLS**.

## Table of Contents

- [Why This Project](#why-this-project)
- [Supported Hardware](#supported-hardware)
- [Scope and Limitations](#scope-and-limitations)
- [Software Components](#software-components)
- [Security and Storage Architecture](#security-and-storage-architecture)
- [Getting Started](#getting-started)
- [Debug in STM32CubeIDE](#debug-in-stm32cubeide)
- [Flash Firmware](#flash-firmware)
- [Provisioning Guides](#provisioning-guides)
- [Run the Examples](#run-the-examples)
- [Required CMSIS Packs](#required-cmsis-packs)
- [Git Submodules](#git-submodules)
- [STM32CubeMX Regeneration Note](#stm32cubemx-regeneration-note)

## Why This Project

This project is intended to provide a clear bring-up path for secure MQTT connectivity on STM32N6 with reusable building blocks:

- FreeRTOS task-based application architecture
- MQTT over TLS integration
- Unified runtime configuration through CLI and KVS
- PKCS#11-based certificate/key handling

## Supported Hardware

- Main board: [STM32N6570-DK](https://www.st.com/en/evaluation-tools/stm32n6570-dk.html)
- Wi-Fi module: [X-NUCLEO-67W61M1](https://www.st.com/en/evaluation-tools/x-nucleo-67w61m1.html)

## Scope and Limitations

### Supported

- ST67_T02-based configuration only
- Single Thing provisioning workflow
- LED example
- Button example

## Software Components

### FreeRTOS, LwIP, MbedTLS,

The application stack is based on:

- FreeRTOS kernel
- LwIP network stack
- MbedTLS TLS/crypto library

### Command Line Interface (CLI)

The device CLI is used for runtime setup and provisioning commands.  
See: [Appli/Common/cli/ReadMe.md](Appli/Common/cli/ReadMe.md)

### PkiObject API

The PkiObject layer handles representation and conversion of certificates/keys used by TLS and provisioning workflows.  
See: [Appli/Common/crypto/ReadMe.md](Appli/Common/crypto/ReadMe.md)

## Security and Storage Architecture

This project keeps the PKCS11/KVS split, with storage on STM32 internal flash.

- `PKCS#11`: cryptographic objects (device key/certificates, CA certificates)
- `KVS`: runtime configuration (MQTT endpoint/port, Wi-Fi credentials, thing name)

```text
                      ┌────────────────────────────┐
                      │   Application Layer        │
                      │  (TLS, MQTT, Wi-Fi setup)  │
                      └──────┬──────────┬──────────┘
                             │          │
                        ┌────▼────┐┌────▼────┐
Manage keys and certs ->│ PKCS#11 ││  KVS    │ <- stores runtime config
                        └────┬────┘└────┬────┘
                             │          │
                      ┌──────▼──────────▼────────┐
                      │ Storage Backend Layer    │
                      ├──────────────────────────┤
                      │ External Flash           │
                      └──────────────────────────┘
```

## Getting Started

### 1. Clone with submodules

```bash
git clone https://github.com/SlimJallouli/stm32n6570_dk_w6x_iot_reference.git --recurse-submodules
```

If already cloned without submodules:

```bash
git submodule update --init --recursive
```

### 2. Quick start with flash and provision using scripts

Use the script-based flow in:

- [bin/readme.md](bin/readme.md)

This is the quick path for bring-up, using prebuilt binaries and a script-based flashing and provisioning process.

### 3. Build in STM32CubeIDE

- Import repository in STM32CubeIDE
- Build both projects:
  - `FSBL`
  - `Appli`

### 4. Debug in STM32CubeIDE

- Set the STM32N6570-DK board to **Dev mode** before starting a debug session.
- Use the provided debug configuration.
- The debug configuration loads:
- `FSBL` in debug mode
  - `Appli` into RAM
- Use **hardware breakpoints** when setting breakpoints in the projects.

### 5. Flash Firmware

- Build `FSBL` in **Release** mode.
- Set the STM32N6570-DK board to **Dev mode**.
- Flash firmware using one of the provided scripts:
  - `flash.ps1` (PowerShell)
  - `flash.sh` (shell)

## Provisioning Guides

Available guides in this repository:

- [Provision and Run with AWS (CLI)](provision_aws_single_cli.md)
- [Provision and Run with AWS (Script)](provision_aws_single_script.md)
- [Provision and Run with test.mosquitto.org](provision_mosquitto.md)

## Run the Examples

After provisioning, run:

- [LED Control Example](Appli/Common/app/led/readme.md)
- [Button Status Example](Appli/Common/app/button/readme.md)

## Required CMSIS Packs

Install these packs before opening the `.ioc` file:

- [ARM.CMSIS-FreeRTOS 11.2.0](https://www.keil.com/pack/ARM.CMSIS-FreeRTOS.11.2.0.pack)
- [ARM.mbedTLS 3.1.1](https://www.keil.com/pack/ARM.mbedTLS.3.1.1.pack)
- [AWS backoffAlgorithm 4.2.0](https://freertos-cmsis-packs.s3.us-west-2.amazonaws.com/AWS.backoffAlgorithm.4.2.0.pack)
- [AWS coreJSON 4.2.0](https://freertos-cmsis-packs.s3.us-west-2.amazonaws.com/AWS.coreJSON.4.2.0.pack)
- [AWS coreMQTT 5.1.0](https://freertos-cmsis-packs.s3.us-west-2.amazonaws.com/AWS.coreMQTT.5.1.0.pack)
- [AWS coreMQTT_Agent 5.1.0](https://freertos-cmsis-packs.s3.us-west-2.amazonaws.com/AWS.coreMQTT_Agent.5.1.0.pack)
- [lwIP 2.3.0](https://www.keil.com/pack/lwIP.lwIP.2.3.0.pack)
- [X-CUBE-ST67W61](https://www.st.com/en/embedded-software/x-cube-st67w61.html)

## Git Submodules

- [corePKCS11](https://github.com/FreeRTOS/corePKCS11)
- [littlefs](https://github.com/littlefs-project/littlefs)
- [tinycbor](https://github.com/intel/tinycbor)

## STM32CubeMX Regeneration Note

After regenerating code with STM32CubeMX, run:

- `update.sh`
