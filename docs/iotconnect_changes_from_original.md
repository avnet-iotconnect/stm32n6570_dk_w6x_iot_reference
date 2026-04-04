# IOTCONNECT Changes From Original Repository

This document is for maintainers reviewing the IOTCONNECT feature branch relative to the original repository. It explains what was added, which original files were changed, and why those changes were necessary.

For end-user onboarding, use [provision_iotconnect.md](../provision_iotconnect.md).

## Scope

The original repository supported AWS IoT Core and Mosquitto. This branch adds a third supported broker flow:

- IOTCONNECT onboarding with device-generated credentials
- IOTCONNECT bootstrap over HTTPS
- IOTCONNECT MQTT runtime using the endpoint, username, topics, and metadata returned by the IOTCONNECT backend

The existing AWS and Mosquitto flows remain in place.

## New Components Added

### IOTCONNECT Runtime

Added:

- [Appli/Common/app/iotconnect](../Appli/Common/app/iotconnect)

Purpose:

- add a dedicated runtime for IOTCONNECT bootstrap, runtime configuration parsing, telemetry publishing, and C2D handling
- keep IOTCONNECT-specific state and protocol handling out of the existing AWS/Mosquitto application paths

What is included there:

- IOTCONNECT client sources
- DRA discovery and identity helpers
- bundled `cJSON`

Why this was added:

- the original repository had no implementation for IOTCONNECT discovery, identity exchange, or topic construction
- IOTCONNECT cannot reuse the original static-broker model because it derives MQTT connection parameters at runtime from HTTPS bootstrap calls

### IOTCONNECT Provisioning Script

Added:

- [bin/provision_iotconnect.ps1](../bin/provision_iotconnect.ps1)

Purpose:

- provide a scripted Windows provisioning flow comparable to the existing AWS and Mosquitto scripts

Why this was added:

- the original repo had no tooling to create an IOTCONNECT device using a certificate generated on the board
- IOTCONNECT onboarding requires collecting the device-generated certificate, guiding the user through the UI, accepting the downloaded `iotcDeviceConfig.json`, and storing IOTCONNECT runtime settings in KVStore

### IOTCONNECT Templates and User Docs

Added or updated:

- [provision_iotconnect.md](../provision_iotconnect.md)
- [docs/provisioning_iotconnect.md](provisioning_iotconnect.md)
- [docs/iotconnect_ui_onboard_quickstart.md](iotconnect_ui_onboard_quickstart.md)
- [IOTCONNECT_Templates/README.md](../IOTCONNECT_Templates/README.md)
- [IOTCONNECT_Templates/stm32n6_w6x_device_template.json](../IOTCONNECT_Templates/stm32n6_w6x_device_template.json)
- [readme.md](../readme.md)
- [bin/readme.md](../bin/readme.md)
- [docs/troubleshooting.md](troubleshooting.md)

Purpose:

- document the new broker flow using the same root-guide plus `docs/` summary pattern already used by AWS and Mosquitto
- document the exact STM32CubeProgrammer version behavior that affected signed-image boot on STM32N6
- document the helper scripts and the IOTCONNECT UI sequence so a user can reproduce onboarding without repo-specific tribal knowledge

Why this was added:

- without these updates, the repo would still describe only AWS and Mosquitto as supported end-user flows
- the IOTCONNECT provisioning path is UI-assisted and needed its own instructions and template examples

## Key Changes to Existing Firmware

### [mqtt_agent_task.c](../Appli/Common/app/mqtt/mqtt_agent_task.c)

What changed:

- added an IOTCONNECT-specific path for MQTT endpoint, client ID, username, and port selection
- restored explicit receive and send timeouts on the MQTT TLS connect path after the upstream sync

Why it changed:

- the original MQTT agent expected endpoint and identity values to come from the existing static broker configuration path
- IOTCONNECT first performs HTTPS bootstrap and then supplies the MQTT endpoint and identity dynamically
- after merging updated upstream `main`, the MQTT connect call in this file was using `0, 0` timeouts again

Observed failure without this change:

- the board completed IOTCONNECT bootstrap but then never completed the MQTT connect path reliably
- the merged branch regressed relative to the earlier working local tree because the nonzero transport timeouts had been lost

### [app_freertos.c](../Appli/Core/Src/app_freertos.c)

What changed:

- added startup of the IOTCONNECT task
- after the upstream sync, added a guard so the upstream PubSub demo task does not start when `broker_type=iotconnect`

Why it changed:

- the original startup code only knew about the existing application tasks
- IOTCONNECT needs its own runtime task
- upstream later added a PubSub demo task, which is valid for its own demo flow but conflicts with the IOTCONNECT runtime because both try to use the shared MQTT agent for different purposes

Observed failure without this change:

- the board would connect to the MQTT broker, then the `PubSub` task would start
- immediately after that, the runtime would show subscribe failures and disconnect/retry behavior
- UART evidence showed `MQTT Agent is connected. Starting the publish subscribe task.` followed by IOTCONNECT subscription failures

### [main.h](../Appli/Core/Inc/main.h)

What changed:

- added the IOTCONNECT task priority and stack size definitions

Why it changed:

- the IOTCONNECT task introduced by this branch needed explicit RTOS configuration
- without these definitions, the application no longer built cleanly once the IOTCONNECT task startup path was added

### [cli_conf.c](../Appli/Common/cli/cli_conf.c)

What changed:

- added handling for IOTCONNECT-related settings and cache keys
- prevented internal IOTCONNECT cache values from being treated as normal user-facing configuration
- aligned IOTCONNECT identity handling with `thing_name`

Why it changed:

- the original CLI config model was designed for AWS/Mosquitto-style user-managed settings
- IOTCONNECT needs a mix of user-visible settings and internal cached runtime state
- exposing the wrong keys as normal config would make the runtime easier to corrupt and harder to reason about

### [cli_pki.c](../Appli/Common/cli/cli_pki.c)

What changed:

- added the on-device key-generation, CSR, and certificate-generation behavior used by the IOTCONNECT provisioning flow
- refined PKCS#11 and non-PKCS#11 cleanup paths

Why it changed:

- the IOTCONNECT onboarding flow depends on generating credentials on the device and then using the resulting certificate in the cloud UI
- the original PKI CLI path was not enough by itself for that onboarding experience

### [kvstore_config.h](../Appli/Common/config/kvstore_config.h)

What changed:

- added IOTCONNECT-specific KV strings and default entries
- added storage for broker selection and IOTCONNECT runtime metadata

Why it changed:

- the original KV schema had no place to persist CPID, ENV, backend identity, or IOTCONNECT cache state
- without these entries, the runtime would have nowhere to persist the bootstrap-derived state required across resets

### [kvstore_nv_littlefs.c](../Appli/Common/kvstore/kvstore_nv_littlefs.c)

What changed:

- added handling so IOTCONNECT runtime state and cached identity material can be stored, refreshed, and invalidated safely

Why it changed:

- IOTCONNECT is not a one-shot static configuration; it has runtime state that may need to be refreshed when the device is reprovisioned
- the original persistence logic did not account for these additional values and invalidation paths

### [PkiObject.c](../Appli/Common/crypto/PkiObject.c)

### [pk_wrap.h](../Appli/Common/include/pk_wrap.h)

What changed:

- extended the PKI object handling needed for device-generated identity and runtime use of that identity

Why it changed:

- IOTCONNECT depends on the board creating and then reusing a local identity for bootstrap and broker authentication
- the original object handling was built around the existing repo use cases and needed extension for this flow

### [mbedtls_transport.c](../Appli/Common/net/lwip_port/mbedtls_transport.c)

What changed:

- added support for the HTTPS bootstrap path used by IOTCONNECT before MQTT startup
- added CA and certificate logging that made runtime validation easier
- after the upstream sync, disabled the `MBEDTLS_TRANSPORT_BROKER_TRACE_PORT` default because it matched the real MQTT broker port `8883`

Why it changed:

- IOTCONNECT requires HTTPS bootstrap to `discovery.iotconnect.io` and the tenant discovery host before MQTT starts
- the original transport path did not cover this exact sequence
- after the upstream merge, the trace-port constant accidentally caused every real broker connection on port `8883` to enter the trace/nonblocking path

Observed failure without the post-sync fix:

- MQTT TLS handshakes would succeed or partially succeed, but the connection then fell into the wrong transport behavior because the debug trace logic was treating the real broker connection as a trace case

### [core_pkcs11_config.h](../Appli/Core/Inc/core_pkcs11_config.h)

### [tls_transport_config.h](../Appli/Core/Inc/tls_transport_config.h)

### [core_pkcs11_pal_utils.c](../Appli/Core/Src/crypto/core_pkcs11_pal_utils.c)

### [core_pkcs11_pal_utils.h](../Appli/Core/Src/crypto/core_pkcs11_pal_utils.h)

What changed:

- added labels, storage mappings, and transport configuration needed by the IOTCONNECT CA chain and device identity flow

Why it changed:

- IOTCONNECT uses both the cloud-provided runtime data and additional CA material that was not present in the original AWS/Mosquitto-only configuration
- the PKCS#11 PAL layer needed to know how to store and retrieve those objects

### [syscalls.c](../Appli/Core/Src/syscalls.c)

What changed:

- added `_gettimeofday()`

Why it changed:

- STM32CubeIDE was classifying the missing syscall warning in a way that made successful links appear as failed builds in the IDE
- this was not IOTCONNECT protocol logic, but it was necessary to keep the build flow readable and reproducible for users testing the branch

## Script Changes

### [run_all.ps1](../bin/run_all.ps1)

What changed:

- added support for `broker_type: "iotconnect"`
- routed the combined flash-and-provision flow to [provision_iotconnect.ps1](../bin/provision_iotconnect.ps1)

Why it changed:

- the original script only knew how to dispatch to the AWS and Mosquitto provisioning flows
- IOTCONNECT needed to plug into the same top-level entry point so users could follow the existing repo workflow

### [flash.ps1](../bin/flash.ps1)

What changed:

- aligned the flashing/signing path with the STM32N6 flow that actually booted on hardware
- removed stale `.sfi`-style assumptions from the maintained flow

Why it changed:

- the working IOTCONNECT validation path used the repo signing flow and staged app binaries directly
- the older experimental packaging assumptions were not the path that was actually validated on hardware

## Post-Sync Validation Notes

After syncing the feature branch with the updated upstream `main`, the IOTCONNECT branch built, flashed, and provisioned successfully but regressed at runtime.

Hardware validation on the merged branch showed three concrete issues:

1. [mqtt_agent_task.c](../Appli/Common/app/mqtt/mqtt_agent_task.c) had lost the explicit TLS connect timeouts.
2. [mbedtls_transport.c](../Appli/Common/net/lwip_port/mbedtls_transport.c) still used `8883` as the broker trace port, which collided with the real MQTT/TLS broker port.
3. [app_freertos.c](../Appli/Core/Src/app_freertos.c) was launching the upstream PubSub demo task even when the broker was IOTCONNECT.

Observed symptom chain:

- Wi-Fi connected successfully
- IOTCONNECT bootstrap over HTTPS succeeded
- broker discovery returned valid MQTT connection parameters
- MQTT/TLS connect reached the broker
- then the merged branch either failed in the transport path or let the PubSub demo interfere with the IOTCONNECT session

These post-sync runtime regressions were reproduced on hardware and corrected locally before this document was updated.

Additional hardware validation was completed after the initial AWS-backed IOTCONNECT verification:

- AWS-backed IOTCONNECT was validated for bootstrap, MQTT connect, telemetry publish, command handling, and ACK publish
- Azure-backed IOTCONNECT was validated for bootstrap, MQTT connect, telemetry publish, command handling, and ACK publish

The Azure validation also required one Azure-specific fix in the bundled IOTCONNECT client:

- [Appli/Common/app/iotconnect/vendor/iotcl.c](../Appli/Common/app/iotconnect/vendor/iotcl.c) now honors MQTT wildcards in the configured subscribe filter so Azure `devices/.../messages/devicebound/#` topics are not silently ignored
- [Appli/Common/app/iotconnect/iotconnect_runtime.c](../Appli/Common/app/iotconnect/iotconnect_runtime.c) now logs successful C2D command receipt and ACK publication so hardware validation is explicit in UART captures

## Files Changed Against `origin/main`

Major feature paths changed relative to the original repository:

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
- [docs/iotconnect_changes_from_original.md](iotconnect_changes_from_original.md)
- [provision_iotconnect.md](../provision_iotconnect.md)
- [readme.md](../readme.md)
