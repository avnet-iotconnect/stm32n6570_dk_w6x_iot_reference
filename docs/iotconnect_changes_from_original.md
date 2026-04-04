# IOTCONNECT Changes From Original Repository

This document summarizes the changes made on top of the original repository to add IOTCONNECT support while preserving the existing AWS and Mosquitto flows.

Use this document as a maintainer-facing change summary. For end-user onboarding, use [provision_iotconnect.md](../provision_iotconnect.md).

## Goal

The original repository supported MQTT-over-TLS provisioning and runtime for:

- AWS IoT Core
- Mosquitto

This branch adds:

- IOTCONNECT onboarding using an on-device generated certificate
- IOTCONNECT runtime bootstrap and MQTT connection support
- Windows provisioning and documentation for the IOTCONNECT flow

The existing AWS and Mosquitto flows remain in place.

## High-Level Additions

### New IOTCONNECT Runtime

Added a new runtime under:

- [Appli/Common/app/iotconnect](../Appli/Common/app/iotconnect)

This runtime is responsible for:

- parsing IOTCONNECT bootstrap and identity responses
- deriving runtime MQTT parameters from the IOTCONNECT backend
- handling IOTCONNECT telemetry and command topic behavior

The vendor import includes:

- IOTCONNECT client sources
- DRA discovery/identity helpers
- bundled `cJSON`

### New Provisioning Flow

Added:

- [bin/provision_iotconnect.ps1](../bin/provision_iotconnect.ps1)

This flow:

- detects the board COM port automatically
- reads or updates the board `thing_name`
- generates the device key pair and certificate on-board
- guides the user through IOTCONNECT device creation with `Use my certificate`
- accepts the downloaded `iotcDeviceConfig.json`
- writes backend, CPID, ENV, and Wi-Fi settings to KVStore
- clears cached IOTCONNECT identity state

### New User-Facing Documentation

Added:

- [provision_iotconnect.md](../provision_iotconnect.md)
- [docs/iotconnect_ui_onboard_quickstart.md](iotconnect_ui_onboard_quickstart.md)
- [IOTCONNECT_Templates/stm32n6_w6x_iot_template_completed.json](../IOTCONNECT_Templates/stm32n6_w6x_iot_template_completed.json)

Updated:

- [readme.md](../readme.md)
- [bin/readme.md](../bin/readme.md)
- [docs/provisioning_iotconnect.md](provisioning_iotconnect.md)
- [docs/troubleshooting.md](troubleshooting.md)
- [IOTCONNECT_Templates/README.md](../IOTCONNECT_Templates/README.md)

## Key Changes to Existing Firmware

### MQTT Agent Integration

Updated:

- [Appli/Common/app/mqtt/mqtt_agent_task.c](../Appli/Common/app/mqtt/mqtt_agent_task.c)

Reason:

- the original MQTT agent expects endpoint and client identity from the existing broker configuration path
- IOTCONNECT first performs HTTPS bootstrap/identity, then derives MQTT endpoint, username, client ID, and topics dynamically

The IOTCONNECT path now:

- uses the IOTCONNECT runtime endpoint instead of the normal KVStore MQTT endpoint
- uses the IOTCONNECT client ID and username
- uses the IOTCONNECT MQTT port

### Task Startup

Updated:

- [Appli/Core/Src/app_freertos.c](../Appli/Core/Src/app_freertos.c)
- [Appli/Core/Inc/main.h](../Appli/Core/Inc/main.h)

Reason:

- add the IOTCONNECT task startup path and task configuration

### CLI and Device Identity

Updated:

- [Appli/Common/cli/cli_conf.c](../Appli/Common/cli/cli_conf.c)
- [Appli/Common/cli/cli_pki.c](../Appli/Common/cli/cli_pki.c)

Reason:

- support on-device key and certificate generation for IOTCONNECT onboarding
- hide internal IOTCONNECT cache keys from normal user configuration
- use `thing_name` as the IOTCONNECT device identity

### KVStore and Runtime State

Updated:

- [Appli/Common/config/kvstore_config.h](../Appli/Common/config/kvstore_config.h)
- [Appli/Common/kvstore/kvstore_nv_littlefs.c](../Appli/Common/kvstore/kvstore_nv_littlefs.c)

Reason:

- add IOTCONNECT runtime keys
- store backend, CPID, ENV, and related IOTCONNECT settings
- refresh or invalidate cached bootstrap/identity state safely

### TLS and PKCS#11 Integration

Updated:

- [Appli/Common/net/lwip_port/mbedtls_transport.c](../Appli/Common/net/lwip_port/mbedtls_transport.c)
- [Appli/Common/crypto/PkiObject.c](../Appli/Common/crypto/PkiObject.c)
- [Appli/Common/include/pk_wrap.h](../Appli/Common/include/pk_wrap.h)
- [Appli/Core/Inc/core_pkcs11_config.h](../Appli/Core/Inc/core_pkcs11_config.h)
- [Appli/Core/Inc/tls_transport_config.h](../Appli/Core/Inc/tls_transport_config.h)
- [Appli/Core/Src/crypto/core_pkcs11_pal_utils.c](../Appli/Core/Src/crypto/core_pkcs11_pal_utils.c)
- [Appli/Core/Src/crypto/core_pkcs11_pal_utils.h](../Appli/Core/Src/crypto/core_pkcs11_pal_utils.h)

Reason:

- support on-device identity creation
- support IOTCONNECT CA usage
- support the TLS/bootstrap path used before MQTT connection

### Build/Runtime Support Fixes

Added or updated:

- [Appli/Core/Src/syscalls.c](../Appli/Core/Src/syscalls.c)

Reason:

- provide `_gettimeofday()` to avoid linker warning classification issues in STM32CubeIDE

## Script-Level Changes

### `run_all.ps1`

Updated:

- [bin/run_all.ps1](../bin/run_all.ps1)

Reason:

- add `broker_type: "iotconnect"`
- route provisioning to `provision_iotconnect.ps1`
- document the IOTCONNECT scripted flow in the `bin` quick start

### `flash.ps1`

Updated:

- [bin/flash.ps1](../bin/flash.ps1)

Reason:

- align flashing behavior with the working STM32N6 signing and boot path used during IOTCONNECT validation

## Documentation Structure

The final structure follows the existing AWS/Mosquitto pattern:

- root `provision_*.md` files are the primary end-user walkthroughs
- `docs/provisioning_*.md` files are concise summary/reference pages
- UI-specific walkthroughs live under `docs/`

For IOTCONNECT specifically:

- [provision_iotconnect.md](../provision_iotconnect.md) is the main walkthrough
- [docs/provisioning_iotconnect.md](provisioning_iotconnect.md) is the short summary
- [docs/iotconnect_ui_onboard_quickstart.md](iotconnect_ui_onboard_quickstart.md) is the UI-specific guide

## Deliberate Non-Goals

These changes were intentionally kept out of the maintained feature diff:

- generated provisioning artifacts
- local `config.json` machine-specific values
- IDE metadata noise
- generated `.bin` staging outputs
- experimental Linux staging flow additions
- stale `.sfi` packaging path

## Files Changed Against `origin/main`

The feature diff against the original repository includes these major paths:

- [Appli/Common/app/iotconnect](../Appli/Common/app/iotconnect)
- [Appli/Common/app/mqtt/mqtt_agent_task.c](../Appli/Common/app/mqtt/mqtt_agent_task.c)
- [Appli/Common/cli/cli_conf.c](../Appli/Common/cli/cli_conf.c)
- [Appli/Common/cli/cli_pki.c](../Appli/Common/cli/cli_pki.c)
- [Appli/Common/config/kvstore_config.h](../Appli/Common/config/kvstore_config.h)
- [Appli/Common/crypto/PkiObject.c](../Appli/Common/crypto/PkiObject.c)
- [Appli/Common/include/pk_wrap.h](../Appli/Common/include/pk_wrap.h)
- [Appli/Common/kvstore/kvstore_nv_littlefs.c](../Appli/Common/kvstore/kvstore_nv_littlefs.c)
- [Appli/Common/net/lwip_port/mbedtls_transport.c](../Appli/Common/net/lwip_port/mbedtls_transport.c)
- [Appli/Core/Inc/core_pkcs11_config.h](../Appli/Core/Inc/core_pkcs11_config.h)
- [Appli/Core/Inc/main.h](../Appli/Core/Inc/main.h)
- [Appli/Core/Inc/tls_transport_config.h](../Appli/Core/Inc/tls_transport_config.h)
- [Appli/Core/Src/app_freertos.c](../Appli/Core/Src/app_freertos.c)
- [Appli/Core/Src/crypto/core_pkcs11_pal_utils.c](../Appli/Core/Src/crypto/core_pkcs11_pal_utils.c)
- [Appli/Core/Src/crypto/core_pkcs11_pal_utils.h](../Appli/Core/Src/crypto/core_pkcs11_pal_utils.h)
- [Appli/Core/Src/syscalls.c](../Appli/Core/Src/syscalls.c)
- [IOTCONNECT_Templates](../IOTCONNECT_Templates)
- [bin/provision_iotconnect.ps1](../bin/provision_iotconnect.ps1)
- [bin/flash.ps1](../bin/flash.ps1)
- [bin/run_all.ps1](../bin/run_all.ps1)
- [bin/readme.md](../bin/readme.md)
- [docs/iotconnect_ui_onboard_quickstart.md](iotconnect_ui_onboard_quickstart.md)
- [docs/provisioning_iotconnect.md](provisioning_iotconnect.md)
- [docs/troubleshooting.md](troubleshooting.md)
- [provision_iotconnect.md](../provision_iotconnect.md)
- [readme.md](../readme.md)
