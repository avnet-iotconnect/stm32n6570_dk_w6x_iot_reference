/*
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 */

#ifndef _KVSTORE_CONFIG_H
#define _KVSTORE_CONFIG_H

#include "kvstore_config_plat.h"

/* -------------------------------- Default Values for Common Attributes -------------------------------- */

/* Default values for attributes. These can be overridden by platform-specific configurations. */
#ifndef THING_NAME_DFLT
    #define THING_NAME_DFLT             "" /* Default Thing Name             */
#endif

#ifndef MQTT_ENDPOINT_DFLT
    #define MQTT_ENDPOINT_DFLT          "" /* Default MQTT Endpoint          */
#endif

#ifndef MQTT_PORT_DFLT
    #define MQTT_PORT_DFLT              8883 /* Default MQTT Port             */
#endif

#ifndef BROKER_TYPE_DFLT
    #define BROKER_TYPE_DFLT            "aws" /* Default broker type          */
#endif

#ifndef IOTCONNECT_CLOUD_DFLT
    #define IOTCONNECT_CLOUD_DFLT       "aws" /* Default IoTConnect backend   */
#endif

#ifndef IOTCONNECT_CPID_DFLT
    #define IOTCONNECT_CPID_DFLT        "" /* Default IoTConnect CPID       */
#endif

#ifndef IOTCONNECT_ENV_DFLT
    #define IOTCONNECT_ENV_DFLT         "" /* Default IoTConnect env        */
#endif

#ifndef IOTCONNECT_DUID_DFLT
    #define IOTCONNECT_DUID_DFLT        "" /* Default IoTConnect DUID       */
#endif

#ifndef IOTCONNECT_APP_MODE_DFLT
    #define IOTCONNECT_APP_MODE_DFLT    "demo" /* Default IoTConnect mode   */
#endif

#ifndef IOTCONNECT_CACHE_VALID_DFLT
    #define IOTCONNECT_CACHE_VALID_DFLT 0U /* Default IoTConnect cache flag */
#endif

#ifndef IOTCONNECT_IDENTITY_JSON_DFLT
    #define IOTCONNECT_IDENTITY_JSON_DFLT "" /* Default IoTConnect identity */
#endif

#ifndef WIFI_SSID_DFLT
    #define WIFI_SSID_DFLT              "" /* Default WiFi SSID             */
#endif

#ifndef WIFI_PASSWORD_DFLT
    #define WIFI_PASSWORD_DFLT          "" /* Default WiFi Password         */
#endif

#ifndef WIFI_SECURITY_DFLT
    #define WIFI_SECURITY_DFLT          "" /* Default WiFi Security         */
#endif

#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
    #ifndef THING_GROUP_NAME_DFLT
        #define THING_GROUP_NAME_DFLT   "" /* Default Thing Group Name      */
        #define PROVISIONED_DEFAULT     0  /* Default Provisioned State     */
    #endif
#endif

#if defined(ST67W6X_NCP)
    #define MQTT_SECURITY_DFLT          0  /* Default MQTT Security for ST67W6X_NCP */
#endif

/* -------------------------------- Key Definitions -------------------------------- */

/* Common keys for all platforms */
#define COMMON_KV_STORE_KEYS                                      \
    CS_CORE_THING_NAME,          /* Thing Name Key             */ \
    CS_CORE_MQTT_ENDPOINT,       /* MQTT Endpoint Key          */ \
    CS_CORE_MQTT_PORT,           /* MQTT Port Key              */ \
    CS_TIME_HWM_S_1970           /* Time High Watermark Key    */

#define IOTCONNECT_KV_STORE_KEYS                                  \
    CS_CORE_BROKER_TYPE,        /* Broker Type Key            */ \
    CS_IOTCONNECT_CLOUD,        /* IoTConnect Cloud Key       */ \
    CS_IOTCONNECT_CPID,         /* IoTConnect CPID Key        */ \
    CS_IOTCONNECT_ENV,          /* IoTConnect Environment Key */ \
    CS_IOTCONNECT_DUID,         /* IoTConnect DUID Key        */ \
    CS_IOTCONNECT_APP_MODE,     /* IoTConnect App Mode Key    */ \
    CS_IOTCONNECT_CACHE_VALID,  /* IoTConnect Cache Flag Key  */ \
    CS_IOTCONNECT_IDENTITY_JSON /* IoTConnect Identity JSON   */

/* Platform-specific keys */
#if defined(ST67W6X_NCP)
typedef enum KvStoreEnum
{
    COMMON_KV_STORE_KEYS,        /* Common Keys                */
    CS_WIFI_SSID,                /* WiFi SSID Key              */
    CS_WIFI_CREDENTIAL,          /* WiFi Credential Key        */
    CS_MQTT_SECURITY,            /* MQTT Security Key          */
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
    CS_PROVISIONED,              /* Provisioned State Key      */
    CS_THING_GROUP_NAME,         /* Thing Group Name Key       */
#endif
    IOTCONNECT_KV_STORE_KEYS,    /* IoTConnect Keys            */
    CS_NUM_KEYS                  /* Total Number of Keys       */
} KVStoreKey_t;

#elif defined(ETHERNET)
typedef enum KvStoreEnum
{
    COMMON_KV_STORE_KEYS,        /* Common Keys                */
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
    CS_PROVISIONED,              /* Provisioned State Key      */
    CS_THING_GROUP_NAME,         /* Thing Group Name Key       */
#endif
    IOTCONNECT_KV_STORE_KEYS,    /* IoTConnect Keys            */
    CS_NUM_KEYS                  /* Total Number of Keys       */
} KVStoreKey_t;

#elif (defined(MXCHIP) || defined(ST67W6X_RCP))
typedef enum KvStoreEnum
{
    COMMON_KV_STORE_KEYS,        /* Common Keys                */
    CS_WIFI_SSID,                /* WiFi SSID Key              */
    CS_WIFI_CREDENTIAL,          /* WiFi Credential Key        */
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
    CS_PROVISIONED,              /* Provisioned State Key      */
    CS_THING_GROUP_NAME,         /* Thing Group Name Key       */
#endif
    IOTCONNECT_KV_STORE_KEYS,    /* IoTConnect Keys            */
    CS_NUM_KEYS                  /* Total Number of Keys       */
} KVStoreKey_t;

#endif

/* -------------------------------- Key-Value Store Strings -------------------------------- */

/* Common strings for all platforms */
#define COMMON_KV_STORE_STRINGS                                   \
    "thing_name",               /* Thing Name String           */ \
    "mqtt_endpoint",            /* MQTT Endpoint String        */ \
    "mqtt_port",                /* MQTT Port                   */ \
    "time_hwm"                  /* Time High Watermark         */

#define IOTCONNECT_KV_STORE_STRINGS                               \
    "broker_type",              /* Broker Type String          */ \
    "iotc_cloud",               /* IoTConnect Cloud String     */ \
    "iotc_cpid",                /* IoTConnect CPID String      */ \
    "iotc_env",                 /* IoTConnect ENV String       */ \
    "iotc_duid",                /* IoTConnect DUID String      */ \
    "iotc_app_mode",            /* IoTConnect App Mode String  */ \
    "iotc_cache_valid",         /* IoTConnect Cache Valid      */ \
    "iotc_identity_json"        /* IoTConnect Identity JSON    */

/* Platform-specific strings */
#if defined(ST67W6X_NCP)
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
#define KV_STORE_STRINGS                                          \
    {                                                             \
        COMMON_KV_STORE_STRINGS,                                  \
        "wifi_ssid",            /* WiFi SSID String            */ \
        "wifi_credential",      /* WiFi Credential String      */ \
        "mqtt_security",        /* MQTT Security String        */ \
        "provision_state",      /* Provisioned State           */ \
        "group_name",           /* Thing Group Name String     */ \
        IOTCONNECT_KV_STORE_STRINGS                               \
    }
#else
#define KV_STORE_STRINGS                                          \
    {                                                             \
        COMMON_KV_STORE_STRINGS,                                  \
        "wifi_ssid",            /* WiFi SSID String            */ \
        "wifi_credential",      /* WiFi Credential String      */ \
        "mqtt_security",        /* MQTT Security String        */ \
        IOTCONNECT_KV_STORE_STRINGS                               \
    }
#endif
#elif defined(ETHERNET)
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
#define KV_STORE_STRINGS                                          \
    {                                                             \
        COMMON_KV_STORE_STRINGS,                                  \
        "provision_state",      /* Provisioned State           */ \
        "group_name",           /* Thing Group Name String     */ \
        IOTCONNECT_KV_STORE_STRINGS                               \
    }
#else
#define KV_STORE_STRINGS                                          \
    {                                                             \
        COMMON_KV_STORE_STRINGS,                                  \
        IOTCONNECT_KV_STORE_STRINGS                               \
    }
#endif

#elif (defined(MXCHIP) || defined(ST67W6X_RCP))
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
#define KV_STORE_STRINGS                                          \
    {                                                             \
        COMMON_KV_STORE_STRINGS,                                  \
        "wifi_ssid",            /* WiFi SSID String            */ \
        "wifi_credential",      /* WiFi Credential String      */ \
        "provision_state",      /* Provisioned State           */ \
        "group_name",           /* Thing Group Name String     */ \
        IOTCONNECT_KV_STORE_STRINGS                               \
    }
#else
#define KV_STORE_STRINGS                                          \
    {                                                             \
        COMMON_KV_STORE_STRINGS,                                  \
        "wifi_ssid",            /* WiFi SSID String            */ \
        "wifi_credential",      /* WiFi Credential String      */ \
        IOTCONNECT_KV_STORE_STRINGS                               \
    }
#endif
#endif

/* -------------------------------- Key-Value Store Defaults -------------------------------- */

/* Common defaults for all platforms */
#define COMMON_KV_STORE_DEFAULTS                                                           \
    KV_DFLT(KV_TYPE_STRING, THING_NAME_DFLT),            /* Default Thing Name          */ \
    KV_DFLT(KV_TYPE_STRING, MQTT_ENDPOINT_DFLT),         /* Default MQTT Endpoint       */ \
    KV_DFLT(KV_TYPE_UINT32, MQTT_PORT_DFLT),             /* Default MQTT Port           */ \
    KV_DFLT(KV_TYPE_UINT32, 0)                           /* Default Time High Watermark */

#define IOTCONNECT_KV_STORE_DEFAULTS                                                       \
    KV_DFLT(KV_TYPE_STRING, BROKER_TYPE_DFLT),           /* Default Broker Type         */ \
    KV_DFLT(KV_TYPE_STRING, IOTCONNECT_CLOUD_DFLT),      /* Default IoTConnect Cloud    */ \
    KV_DFLT(KV_TYPE_STRING, IOTCONNECT_CPID_DFLT),       /* Default IoTConnect CPID     */ \
    KV_DFLT(KV_TYPE_STRING, IOTCONNECT_ENV_DFLT),        /* Default IoTConnect ENV      */ \
    KV_DFLT(KV_TYPE_STRING, IOTCONNECT_DUID_DFLT),       /* Default IoTConnect DUID     */ \
    KV_DFLT(KV_TYPE_STRING, IOTCONNECT_APP_MODE_DFLT),   /* Default IoTConnect App Mode */ \
    KV_DFLT(KV_TYPE_UINT32, IOTCONNECT_CACHE_VALID_DFLT),/* Default IoTConnect Cache    */ \
    KV_DFLT(KV_TYPE_STRING, IOTCONNECT_IDENTITY_JSON_DFLT) /* Default Identity JSON     */

/* Defaults for ST67W6X_NCP platform */
#if defined(ST67W6X_NCP)
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
#define KV_STORE_DEFAULTS                                                                  \
    {                                                                                      \
        COMMON_KV_STORE_DEFAULTS,                                                          \
        KV_DFLT(KV_TYPE_STRING, WIFI_SSID_DFLT),         /* Default WiFi SSID         */   \
        KV_DFLT(KV_TYPE_STRING, WIFI_PASSWORD_DFLT),     /* Default WiFi Password     */   \
        KV_DFLT(KV_TYPE_UINT32, MQTT_SECURITY_DFLT),     /* Default MQTT Security     */   \
        KV_DFLT(KV_TYPE_UINT32, PROVISIONED_DEFAULT),    /* Default Provisioned State */   \
        KV_DFLT(KV_TYPE_STRING, THING_GROUP_NAME_DFLT),  /* Default Thing Group Name  */   \
        IOTCONNECT_KV_STORE_DEFAULTS                                                          \
    }
#else
#define KV_STORE_DEFAULTS                                                                  \
    {                                                                                      \
        COMMON_KV_STORE_DEFAULTS,                                                          \
        KV_DFLT(KV_TYPE_STRING, WIFI_SSID_DFLT),         /* Default WiFi SSID         */   \
        KV_DFLT(KV_TYPE_STRING, WIFI_PASSWORD_DFLT),     /* Default WiFi Password     */   \
        KV_DFLT(KV_TYPE_UINT32, MQTT_SECURITY_DFLT),     /* Default MQTT Security     */   \
        IOTCONNECT_KV_STORE_DEFAULTS                                                          \
    }
#endif
/* Defaults for ETHERNET platform */
#elif defined(ETHERNET)
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
#define KV_STORE_DEFAULTS                                                                  \
    {                                                                                      \
        COMMON_KV_STORE_DEFAULTS,                                                          \
        KV_DFLT(KV_TYPE_UINT32, PROVISIONED_DEFAULT),    /* Default Provisioned State */   \
        KV_DFLT(KV_TYPE_STRING, THING_GROUP_NAME_DFLT),  /* Default Thing Group Name  */   \
        IOTCONNECT_KV_STORE_DEFAULTS                                                          \
    }
#else
#define KV_STORE_DEFAULTS                                                                  \
    {                                                                                      \
        COMMON_KV_STORE_DEFAULTS,                                                          \
        IOTCONNECT_KV_STORE_DEFAULTS                                                          \
    }
#endif

/* Defaults for MXCHIP platform */
#elif (defined(MXCHIP) || defined(ST67W6X_RCP))
#if defined(DEMO_AWS_FLEET_PROVISION) && !defined(__USE_STSAFE__)
#define KV_STORE_DEFAULTS                                                                  \
    {                                                                                      \
        COMMON_KV_STORE_DEFAULTS,                                                          \
        KV_DFLT(KV_TYPE_STRING, WIFI_SSID_DFLT),         /* Default WiFi SSID         */   \
        KV_DFLT(KV_TYPE_STRING, WIFI_PASSWORD_DFLT),     /* Default WiFi Password     */   \
        KV_DFLT(KV_TYPE_UINT32, PROVISIONED_DEFAULT),    /* Default Provisioned State */   \
        KV_DFLT(KV_TYPE_STRING, THING_GROUP_NAME_DFLT),  /* Default Thing Group Name  */   \
        IOTCONNECT_KV_STORE_DEFAULTS                                                          \
    }
#else
#define KV_STORE_DEFAULTS                                                                  \
    {                                                                                      \
        COMMON_KV_STORE_DEFAULTS,                                                          \
        KV_DFLT(KV_TYPE_STRING, WIFI_SSID_DFLT),         /* Default WiFi SSID         */   \
        KV_DFLT(KV_TYPE_STRING, WIFI_PASSWORD_DFLT),     /* Default WiFi Password     */   \
        IOTCONNECT_KV_STORE_DEFAULTS                                                          \
    }
#endif
#endif

#endif /* _KVSTORE_CONFIG_H */
