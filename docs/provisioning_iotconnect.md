# /IOTCONNECT Provisioning

This guide covers the single `/IOTCONNECT` onboarding flow used by this project.

The flow is:
- the board generates its own private key and device certificate
- you create the device in `/IOTCONNECT` using that certificate
- you download the device JSON from `/IOTCONNECT`
- you paste that JSON into the provisioning script
- the script applies the settings, resets the board, and asks you to confirm the device is connected

Unlike the older flow, this project no longer expects you to import a device certificate ZIP or a private key file from the host.

## Supported Runtime Mode

- `/IOTCONNECT` provisioning uses the LED/button demo mode for this project

## Identity Rules

- `/IOTCONNECT` DUID is derived from the board `thing_name`
- when you create the device in `/IOTCONNECT`, use the board `thing_name` as the Unique ID

## Prerequisites

- Board connected through ST-LINK Virtual COM
- Internet access
- Wi-Fi credentials in `bin/config.json`
- `/IOTCONNECT` device JSON after device creation

## Recommended `config.json`

```json
{
  "broker_type": "iotconnect",
  "wifi_ssid": "YOUR_WIFI",
  "wifi_credential": "YOUR_PASSWORD"
}
```

Field notes:

- `portName` is optional; if set, provisioning uses that exact ST-LINK VCP
- the script parses backend, CPID, ENV, UID, DID, and discovery URL from the pasted `/IOTCONNECT` device JSON
- MQTT and IOTCONNECT DRA root CAs are built into the provisioning script

## Recommended Flow (`run_all.ps1`)

1. Edit `bin/config.json` with:
   - `"broker_type": "iotconnect"`
   - Wi-Fi credentials
2. Run:

```powershell
cd bin
.\run_all.ps1
```

Behavior:

 - firmware is flashed
 - the board generates `tls_key_priv`, `tls_key_pub`, and `tls_cert`
 - the script shows the certificate in the terminal and saves a copy locally
 - you create the device in `/IOTCONNECT`
 - you paste the downloaded device JSON into the script
 - the script imports built-in CA certificates
 - the script sets backend, CPID, ENV, app mode, and Wi-Fi
 - the script refreshes the internal `/IOTCONNECT` identity cache state
 - the script resets the board and asks you to confirm the device is connected

## Provisioning Only

If firmware is already flashed:

```powershell
.\provision_iotconnect.ps1
```

The script:
- connects to the board CLI
- reads the board `thing_name`
- lets you keep that `thing_name` or replace it
- generates `tls_key_priv` and `tls_key_pub`
- generates `tls_cert`
- saves the device certificate PEM under:
  - `bin\generated_iotconnect_identity\<thing_name>.iotconnect.cert.pem`
- tells you exactly what to enter in the `/IOTCONNECT` New Device screen:
  - `Unique Id = thing_name`
  - `Device Name = thing_name`
  - select the chosen Entity
  - select the chosen Template
  - select only `Use my certificate`
  - paste the printed PEM into `Certificate Text`
- waits for you to download the device JSON and paste it into the terminal
- verifies that the device JSON UID and DID match the board `thing_name`
- validates the discovery URL for the selected backend
- imports built-in MQTT root CA
- imports built-in IOTCONNECT DRA CA
- verifies the on-device certificate and public key
- stores:
  - `broker_type`
  - `iotc_cloud`
  - `iotc_cpid`
  - `iotc_env`
  - Wi-Fi settings
- refreshes internal `/IOTCONNECT` cached identity state
- commits config and resets the board

## Cache Behavior

Cache invalidation is automatic provisioning behavior.

The script automatically refreshes the internal `/IOTCONNECT` identity cache during provisioning.
Those runtime cache-control keys are implementation details, not user-facing configuration settings.

## Notes

- the private key stays on the device
- the board-generated certificate is EC-based
- the firmware uses `thing_name` as the `/IOTCONNECT` device identity
- if more than one ST-LINK COM port is connected, set `portName` in `config.json` or `STM32_CLI_PORT`
- for the UI-oriented walkthrough, see [iotconnect_ui_onboard_quickstart.md](../docs/iotconnect_ui_onboard_quickstart.md)

## Related Files

- [bin/provision_iotconnect.ps1](../bin/provision_iotconnect.ps1)
- [docs/iotconnect_ui_onboard_quickstart.md](iotconnect_ui_onboard_quickstart.md)
- [IOTCONNECT_Templates/stm32n6_w6x_iot_template_completed.json](../IOTCONNECT_Templates/stm32n6_w6x_iot_template_completed.json)
