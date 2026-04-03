# /IOTCONNECT Getting Started

This is the shortest path to bring the STM32N6570-DK project up on `/IOTCONNECT`.

Use this guide when you want a working device quickly.
Use [docs/provisioning_iotconnect.md](docs/provisioning_iotconnect.md) when you need the full flow details.

## What This Flow Does

- uses `broker_type: "iotconnect"` at runtime
- generates the device certificate on the board
- uses the board `thing_name` as the `/IOTCONNECT` device identity
- imports built-in MQTT root CA and IOTCONNECT DRA CA certificates
- parses backend, CPID, and ENV from the `/IOTCONNECT` device JSON
- boots the device into the `/IOTCONNECT` runtime

## Prerequisites

- STM32N6570-DK board connected over ST-LINK USB
- STM32CubeProgrammer installed
- Internet access
- Wi-Fi SSID and password

## 1. Update `bin/config.json`

Edit [bin/config.json](/c:/dev/slim/stm32n6570_dk_w6x_iot_reference/bin/config.json):

```json
{
  "broker_type": "iotconnect",
  "wifi_ssid": "YOUR_WIFI",
  "wifi_credential": "YOUR_PASSWORD"
}
```

Notes:

- the `/IOTCONNECT` flow uses the LED/button demo mode

## 2. Flash And Enroll

From [bin](/c:/dev/slim/stm32n6570_dk_w6x_iot_reference/bin):

```powershell
.\run_all.ps1
```

The IoTConnect step:
- generates the keypair and certificate on-device
- saves the PEM certificate locally
- prints the board `thing_name`
- tells you exactly what to enter in the `/IOTCONNECT` Create Device page
- waits for you to paste the downloaded device JSON back into the terminal

## 3. Create The Device In `/IOTCONNECT`

1. Create the device in `/IOTCONNECT`
2. Use the generated board certificate
3. Use the board `thing_name` as both the Unique ID and Device Name
4. Select the `STM32N6 W6X LED and Button Demo` template
5. Select `Use my certificate`
6. Paste the printed certificate into `Certificate Text`
7. Click `Save & View`
8. Download the device JSON, typically `iotcDeviceConfig.json`

## 4. Paste The Device JSON

When the script asks for the device JSON:

1. paste the full downloaded JSON
2. type `ENDJSON` on its own line

The script will then:

1. parse the `/IOTCONNECT` JSON
2. verify the JSON UID and DID match the board `thing_name`
3. import built-in CA certificates
4. write `/IOTCONNECT` runtime settings into KVStore
5. clear the cached `/IOTCONNECT` identity
6. reset the board

## 6. What Happens On First Boot

On the first `/IOTCONNECT` boot, firmware will:

- connect to Wi-Fi
- call `/IOTCONNECT` discovery and identity over HTTPS
- derive MQTT host, client ID, username, and topics from the identity response
- connect to `/IOTCONNECT` MQTT on port `8883`

## 7. Choose An App Mode

`demo` mode:
- receives LED commands from `/IOTCONNECT`
- publishes button and LED state telemetry
- sends command acknowledgements

`sample` mode:
- publishes periodic sample telemetry
- handles generic command and OTA ACK flows

## 8. Useful Command

```powershell
.\provision_iotconnect.ps1
```

## 9. Quick Checks

If the device does not connect:

- verify Wi-Fi credentials in [bin/config.json](/c:/dev/slim/stm32n6570_dk_w6x_iot_reference/bin/config.json)
- verify the `/IOTCONNECT` JSON is the correct `iotcDeviceConfig.json` for the created device
- verify the `/IOTCONNECT` device Unique ID matches the board `thing_name`
- verify the serial COM port is not busy

## Related Files

- [bin/provision_iotconnect.ps1](/c:/dev/slim/stm32n6570_dk_w6x_iot_reference/bin/provision_iotconnect.ps1)
- [docs/provisioning_iotconnect.md](/c:/dev/slim/stm32n6570_dk_w6x_iot_reference/docs/provisioning_iotconnect.md)
- [docs/iotconnect_ui_onboard_quickstart.md](/c:/dev/slim/stm32n6570_dk_w6x_iot_reference/docs/iotconnect_ui_onboard_quickstart.md)
- [bin/readme.md](/c:/dev/slim/stm32n6570_dk_w6x_iot_reference/bin/readme.md)
