#ifndef IOTCONNECT_RUNTIME_H
#define IOTCONNECT_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "core_mqtt.h"

typedef enum AppBrokerType
{
    APP_BROKER_AWS = 0,
    APP_BROKER_MOSQUITTO,
    APP_BROKER_IOTCONNECT
} AppBrokerType_t;

typedef enum IoTConnectAppMode
{
    IOTCONNECT_APP_MODE_DEMO = 0,
    IOTCONNECT_APP_MODE_SAMPLE
} IoTConnectAppMode_t;

AppBrokerType_t xAppGetBrokerType( void );

BaseType_t xAppIsIoTConnectBroker( void );

BaseType_t xIoTConnectRuntimeIsReady( void );

IoTConnectAppMode_t xIoTConnectGetAppMode( void );

const char * pcIoTConnectMqttEndpoint( void );

const char * pcIoTConnectMqttClientId( void );

const char * pcIoTConnectMqttUserName( void );

uint16_t usIoTConnectMqttPort( void );

MQTTStatus_t xIoTConnectPublishPayload( const char * pcTopic,
                                        const char * pcPayload,
                                        MQTTQoS_t xQoS,
                                        bool xRetain );

void vIoTConnectStartupTask( void * pvParameters );

#endif /* IOTCONNECT_RUNTIME_H */
