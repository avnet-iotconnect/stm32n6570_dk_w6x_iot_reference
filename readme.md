
# STM32N6570-DK and ST67 IoT Reference integration
## 1. Introduction
This project demonstrates how to integrate modular [ FreeRTOS kernel ](https://www.freertos.org/RTOS.html) and [ libraries ](https://www.freertos.org/libraries/categories.html). The project is pre-configured to run on the [ STM32N6570-DK ](https://www.st.com/en/evaluation-tools/stm32n6570-dk.htmll) board with [ST67W611M1](https://www.st.com/content/st_com/en/campaigns/st67w-wifi6-bluetooth-thread-module-z13.html).

The *Project* is a [Non-TrustZone](https://www.arm.com/technologies/trustzone-for-cortex-m) project which  demonstrate connecting to MQTT broker over 2.4 GHz Wi-Fi.

The project can connect to any MQTT broker including AWS. It has been tested with [broker.emqx.io](https://www.emqx.com/en/mqtt/public-mqtt5-broker), [test.mosquitto.org](https://test.mosquitto.org/) and [amazonaws.com](https://aws.amazon.com/)

The demo project connect to MQTT broker via the [ST67W611M1](https://www.st.com/content/st_com/en/campaigns/st67w-wifi6-bluetooth-thread-module-z13.html) Wi-Fi module and use the [CoreMQTT-Agent](https://github.com/FreeRTOS/coreMQTT-Agent) library to share a single MQTT connection among multiple tasks.

## 2. MQTT Demos
The following demos can be used with any MQTT broker.

* Publish and Subscribe
* EnvironmentSensor
* MotionSensors

## 3. AWS IoT Core Demo Tasks
Demonstration tasks for the following AWS services:
* [AWS IoT Device Shadow](https://docs.aws.amazon.com/iot/latest/developerguide/iot-device-shadows.html)
* [AWS IoT OTA Update](https://docs.aws.amazon.com/freertos/latest/userguide/freertos-ota-dev.html)
* [AWS IoT Jobs](https://docs.aws.amazon.com/iot/latest/developerguide/iot-jobs.html)
* [MQTT File Delivery](https://docs.aws.amazon.com/iot/latest/developerguide/mqtt-based-file-delivery.html)


## 4. Key Software Components
### Mbedtls 3.1.1 TLS and Cryptography library
See [ MbedTLS ](https://www.keil.arm.com/packs/mbedtls-arm/versions/) for details.

### Command Line Interface (CLI)
The CLI interface located in the Common/cli directory is used to provision the device. It also provides other Unix-like utilities. See [Common/cli](Common/cli/ReadMe.md) for details.

### Key-Value Store
The key-value store located in the Common/kvstore directory is used to store runtime configuration values in STM32's internal flash memory.
See [Common/kvstore](Common/kvstore/ReadMe.md) for details.

### PkiObject API
The PkiObject API takes care of some of the mundane tasks in converting between different representations of cryptographic objects such as public keys, private keys, and certificates. See [Common/crypto](Common/crypto/ReadMe.md) for details.

## 5. Get started with the project

### 5.1 Cloning the Repository
To clone using HTTPS:
```
git clone https://github.com/SlimJallouli/stm32n6570_dk_w6x_iot_reference.git --recurse-submodules
```

If you have downloaded the repo without using the `--recurse-submodules` argument, you should run:

```
git submodule update --init --recursive
```

### 5.2 Build the project
* Import the project with [STM32CubeIDE](http://www.st.com/stm32cubeide)
* Build the FSBL and Appli projects


![alt text](<assets/Screenshot 2025-05-19 113547.png>)\

### 5.3 Flash the project

* Set the boot pins to DEV mode (Add picture)

* Powercycle the board

* Use the flash.sh script to flash the FSBL and Application to your board.

* Set the boot pins to Flash mode.

* Reset the board

## 6. Connect the board to generic MQTT Broker

Use a serial terminal like [TeraTerm](https://teratermproject.github.io/index-en.html) or [Web based Serial terminal](https://googlechromelabs.github.io/serial-terminal/)

Connect to the board 115200, 8 bits, 1 Stop bit, No parity

Use the CLI to connect the board to the MQTT Broker

* Connect to [broker.emqx.io](https://www.emqx.com/en/mqtt/public-mqtt5-broker)
```
conf set thing_name <YourThingName>
conf set wifi_ssid <YourWiFiSSID>
conf set wifi_credential <YourWiFiPassword>
conf set mqtt_endpoint  broker.emqx.io
conf set mqtt_port 1883
conf set mqtt_security 0
conf commit
reset
```

* Connect to [test.mosquitto.org](https://test.mosquitto.org/)
```
conf set thing_name <YourThingName>
conf set wifi_ssid <YourWiFiSSID>
conf set wifi_credential <YourWiFiPassword>
conf set mqtt_endpoint  test.mosquitto.org
conf set mqtt_port 1883
conf set mqtt_security 0
conf commit
reset
```

![alt text](<assets/Screenshot 2025-05-08 163343.png>)

## 7. Connect to AWS

### 7.1 Delete cerrts on ST67

Use the serial termina to delete any certs present on ST67

* Use **w6x_fs ls** to list all the files present on ST67 file system

* Use **w6x_fs rm < filename >** to delete a file


### 7.1 [Single Thing Provisioning](https://docs.aws.amazon.com/iot/latest/developerguide/single-thing-provisioning.html)

[Single Thing Provisioning](https://docs.aws.amazon.com/iot/latest/developerguide/single-thing-provisioning.html), is a method used to provision individual IoT devices in AWS IoT Core. This method is ideal for scenarios where you need to provision devices one at a time.

In this method you have two options. Automated using Python script or manual.

1. [Provision automatically with provision.py](https://github.com/FreeRTOS/iot-reference-stm32u5/blob/main/Getting_Started_Guide.md#option-8a-provision-automatically-with-provisionpy)

This method involves using a Python script [(provision.py)](https://github.com/FreeRTOS/iot-reference-stm32u5/blob/main/tools/provision.py) to automate the onboarding process of IoT devices to AWS IoT Core. It simplifies the process by handling the device identity creation, registration, and policy attachment automatically. follow this [link](https://github.com/FreeRTOS/iot-reference-stm32u5/blob/main/Getting_Started_Guide.md#option-8a-provision-automatically-with-provisionpy) for instructions

2. [Provision Manually via CLI](https://github.com/FreeRTOS/iot-reference-stm32u5/blob/main/Getting_Started_Guide.md#option-8b-provision-manually-via-cli)

This method requires manually provisioning devices using the AWS Command Line Interface (CLI). It involves creating device identities, registering them with AWS IoT Core, and attaching the necessary policies for device communication. Follow this  [link](https://github.com/FreeRTOS/iot-reference-stm32u5/blob/main/Getting_Started_Guide.md#option-8b-provision-manually-via-cli) for instructions.

## 8. CMSIS Packs

If you need to regenerate the project with STM32CubeMX, then you need to dowload and install the following CMSIS packs.

[CMSIS-FreeRTOS 11.1.0](https://www.keil.com/pack/ARM.CMSIS-FreeRTOS.11.1.0.pack)

[mbedTLS 3.1.1](https://www.keil.com/pack/ARM.mbedTLS.3.1.1.pack)

[AWS_IoT_Over-the-air_Update 5.0.1](https://d1pm0k3vkcievw.cloudfront.net/AWS.AWS_IoT_Over-the-air_Update.5.0.1.pack)

[AWS_IoT_Device_Shadow 5.0.1](https://d1pm0k3vkcievw.cloudfront.net/AWS.AWS_IoT_Device_Shadow.5.0.1.pack)

[backoffAlgorithm 4.1.1](https://d1pm0k3vkcievw.cloudfront.net/AWS.backoffAlgorithm.4.1.1.pack)

[coreJSON 4.1.1](https://d1pm0k3vkcievw.cloudfront.net/AWS.coreJSON.4.1.1.pack)

[coreMQTT 5.0.1](https://d1pm0k3vkcievw.cloudfront.net/AWS.coreMQTT.5.0.1.pack)

[coreMQTT_Agent 5.0.1](https://d1pm0k3vkcievw.cloudfront.net/AWS.coreMQTT_Agent.5.0.1.pack)


## 7. Git submodules

[corePKCS11](https://github.com/FreeRTOS/corePKCS11)

[littlefs](https://github.com/littlefs-project/littlefs)

[tinycbor](https://github.com/intel/tinycbor)


## 8. Generate the project using STM32CubeMX

After making changes with STM32CubeMX, be sure to run the **update.sh** script. Failure to do so will result in build errors.
