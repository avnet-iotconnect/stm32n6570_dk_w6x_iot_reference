# IOTCONNECT Provisioning

This guide covers the IOTCONNECT onboarding flow used by this project.

## Prerequisites

- Board connected through ST-LINK Virtual COM
- Internet access
- Wi-Fi credentials in [`bin/config.json`](../bin/config.json)
- IOTCONNECT device JSON downloaded after device creation

## Recommended Flow (`run_all.ps1`)

1. Edit [`bin/config.json`](../bin/config.json):
   - `"broker_type": "iotconnect"`
   - Wi-Fi credentials
2. If you built from STM32CubeIDE first, run:

```powershell
cd ..\bin
.\copy_hex_from_project.ps1
```

3. Run:

```powershell
cd ..\bin
.\run_all.ps1
```

The script flow:

- flashes firmware
- runs IOTCONNECT provisioning
- generates the device certificate on-board
- walks you through IOTCONNECT device creation
- imports built-in CA certificates
- stores backend, CPID, ENV, and Wi-Fi settings
- resets the device

## Provisioning Only

If firmware is already flashed:

```powershell
cd ..\bin
.\provision_iotconnect.ps1
```

## Notes

- The board `thing_name` is used as the IOTCONNECT device identity.
- The private key stays on the device.
- The IOTCONNECT flow uses the LED/button demo mode.
- For the full step-by-step workflow, see [provision_iotconnect.md](../provision_iotconnect.md).
- For the UI-driven walkthrough, see [iotconnect_ui_onboard_quickstart.md](iotconnect_ui_onboard_quickstart.md).
