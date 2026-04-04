# IOTCONNECT Provisioning for STM32N6570-DK (Script Method)

This guide explains how to provision a single STM32N6570-DK device on IOTCONNECT using the automated `provision_iotconnect.ps1` workflow.

Use this flow when you want the device to:

- generate its own certificate on the board
- register in IOTCONNECT with `Use my certificate`
- download the IOTCONNECT device JSON
- configure the runtime without reflashing firmware

## Supported Build Configurations

| Build Config | Provisioning Method |
|---|---|
| `ST67_T02_Single` | IOTCONNECT device-generated certificate onboarding |

## 1. Hardware Setup

- Connect the Wi-Fi module to the `Arduino` connector.
- Connect ST-Link USB to your PC for power, flashing, and debugging.

## 2. Prerequisites

Before provisioning:

1. Build the application and FSBL in STM32CubeIDE.
2. From [bin](bin), run:

```powershell
.\copy_hex_from_project.ps1
```

3. Confirm [bin/config.json](bin/config.json) contains at least:

```json
{
  "broker_type": "iotconnect",
  "wifi_ssid": "YOUR_WIFI",
  "wifi_credential": "YOUR_PASSWORD"
}
```

4. Close any serial terminal connected to the board before running the script.

## 3. Run Automated Provisioning

From [bin](bin):

```powershell
.\provision_iotconnect.ps1
```

The script:

- detects the ST-LINK virtual COM port automatically
- reads the board `thing_name`
- lets you keep or update the `thing_name`
- generates `tls_key_priv`, `tls_key_pub`, and `tls_cert` on the device
- prints the device certificate in PEM format for IOTCONNECT onboarding
- waits for you to paste the downloaded IOTCONNECT device JSON
- configures the runtime and resets the board

## 4. Create the Device in IOTCONNECT

When the script prints the onboarding instructions:

1. Open the IOTCONNECT `Create Device` page.
2. Use the board `thing_name` as both `Unique Id` and `Device Name`.
3. Select the appropriate `Entity`.
4. Select template `STM32N6 W6X LED and Button Demo`.
5. In `Device certificate`, select only `Use my certificate`.
6. Paste the device certificate shown by the script into `Certificate Text`.
7. Click `Save & View`.
8. On the device page, click the document and gear icon to download the device JSON.

The downloaded JSON looks similar to:

```json
{
  "ver": "2.1",
  "pf": "aws",
  "cpid": "97FF86E8728645E9B89F7B07977E4B15",
  "env": "poc",
  "uid": "stm32n6-0031003B4236500E0036324E",
  "did": "stm32n6-0031003B4236500E0036324E",
  "at": 7,
  "disc": "https://awsdiscovery.iotconnect.io"
}
```

## 5. Paste the IOTCONNECT Device JSON

When the script prompts for the device JSON:

1. Paste the full JSON content into the terminal.
2. Type `ENDJSON` on its own line.

The script then:

- validates `pf`, `cpid`, `env`, `uid`, `did`, and `disc`
- verifies `uid` matches the board `thing_name`
- imports the built-in MQTT root CA and IOTCONNECT DRA CA
- writes the IOTCONNECT runtime configuration to KVStore
- clears cached IOTCONNECT identity state
- resets the board

## 6. Monitor First Boot

After reset:

- wait up to 60 seconds for the device to connect
- monitor the ST-LINK VCP in a serial terminal at `115200 8N1`
- confirm the device appears connected in IOTCONNECT

If the device does not connect:

- verify Wi-Fi credentials in [bin/config.json](bin/config.json)
- verify the pasted JSON belongs to the created device
- verify `Unique Id` matches the board `thing_name`
- rerun `.\provision_iotconnect.ps1` to reprovision without reflashing

## 7. Reprovision Without Reflashing

If the device is already flashed and you only need to reprovision:

```powershell
.\provision_iotconnect.ps1
```

Use this to:

- regenerate or replace the device certificate
- create a new IOTCONNECT device
- paste a new `iotcDeviceConfig.json`
- update Wi-Fi or IOTCONNECT runtime settings without reflashing firmware

## 8. Related Files

- [bin/provision_iotconnect.ps1](bin/provision_iotconnect.ps1)
- [bin/readme.md](bin/readme.md)
- [docs/provisioning_iotconnect.md](docs/provisioning_iotconnect.md)
- [docs/iotconnect_ui_onboard_quickstart.md](docs/iotconnect_ui_onboard_quickstart.md)

---

[Back to Main README](readme.md)
