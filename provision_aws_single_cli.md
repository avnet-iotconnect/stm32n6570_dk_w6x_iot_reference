# AWS IoT Core Single-Device Provisioning for STM32N6570-DK (CLI Method)

This guide explains how to provision a **single STM32N6570-DK device** on **AWS IoT Core** using the on-device CLI (manual method). It is intended for developers who want direct control over key generation, certificate registration, and runtime MQTT/TLS configuration.

See AWS background: [Single Thing Provisioning](https://docs.aws.amazon.com/iot/latest/developerguide/single-thing-provisioning.html).

## 1. Connect a Serial Terminal

Open a serial terminal (Tera Term, PuTTY, or [web serial terminal](https://googlechromelabs.github.io/serial-terminal/)) with:

- Baud: `115200`
- Data bits: `8`
- Stop bits: `1`
- Parity: `None`

![Terminal Configuration](assets/TeraTerm_Config.png)

## 2. Get the Device Thing Name

Retrieve the generated Thing/device identifier from the board CLI:

```bash
conf get
```

Save this value for AWS registration and MQTT topic filtering.

![CLI conf get](assets/conf_get.png)

## 3. Generate Device Key Pair

Run:

```bash
pki generate key
```

This generates an ECC key pair via MbedTLS and PKCS#11 and stores it in internal flash (LFS/PKCS#11 stack).

![Generate Key](assets/pki_generate_key.png)

## 4. Generate Device Certificate

Run:

```bash
pki generate cert
```

This creates a self-signed certificate from the device key pair and prints it in PEM format.

- Copy the PEM output and save it as `cert.pem`.

![Generate Certificate](assets/pki_generate_cert.png)

## 5. Register Device in AWS IoT Core

### a) Open AWS IoT Console

1. Go to [AWS IoT Core Console](https://console.aws.amazon.com/iot).
2. Select `Manage` -> `Things`.
3. Click `Create things`.

### b) Create Single Thing

1. Select `Create a single thing`.
2. Enter the Thing Name from step 3.
3. Click `Next`.

### c) Upload Device Certificate

1. Select `Use my certificate`.
2. For CA certificate, select `CA is not registered with AWS IoT`.
3. Upload `cert.pem`.
4. Click `Next`.

### d) Attach Policy

1. Create a policy (example name: `AllowAllDev`).
2. Use the following policy document:

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": "iot:*",
      "Resource": "*"
    }
  ]
}
```

3. Attach the policy to the Thing certificate.

### e) Finish

Create the Thing. The device is now registered for AWS IoT Core TLS authentication.

## 6. Download AWS Root CA

```bash
wget https://www.amazontrust.com/repository/SFSRootCAG2.pem
```

Or download manually: [SFSRootCAG2.pem](https://www.amazontrust.com/repository/SFSRootCAG2.pem)

## 7. Import AWS Root CA to STM32

On the board CLI:

```bash
pki import cert root_ca_cert
```

Paste the full `SFSRootCAG2.pem` contents (including `BEGIN CERTIFICATE` and `END CERTIFICATE`) into the terminal.

![Import Root CA](assets/pki_import_root_ca.png)

## 8. Set Runtime MQTT and Network Configuration

Set AWS endpoint:

```bash
conf set mqtt_endpoint <your-aws-endpoint>
```

Set MQTT port:

```bash
conf set mqtt_port 8883
```

Set Wi-Fi:

```bash
conf set wifi_ssid <YourSSID>
conf set wifi_credential <YourPASSWORD>
```

Commit and verify:

```bash
conf commit
conf get
```

![AWS conf commit](assets/aws_conf_commit.png)
![AWS conf get](assets/aws_conf_get.png)

## 9. Reset and Connect

```bash
reset
```

After reboot, the device connects using the new TLS assets and MQTT settings.

![MQTT Connection](assets/mqtt_connection.png)

## 10. Monitor MQTT Messages in AWS

1. Open [AWS IoT Core Console](https://console.aws.amazon.com/iot).
2. Select `MQTT test client`.
3. Subscribe to your device topic (for example `<thing-name>/#` or `#`).
4. Verify telemetry in real time.

![AWS MQTT Test Client](assets/aws_mqtt_test_client.png)
