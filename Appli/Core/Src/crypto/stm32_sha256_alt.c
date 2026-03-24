#define MBEDTLS_ALLOW_PRIVATE_ACCESS

#include "mbedtls/sha256.h"
#include "main.h"
#include "logging_levels.h"
#define LOG_LEVEL    LOG_INFO
#include "logging.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <string.h>

extern HASH_HandleTypeDef hhash;

static SemaphoreHandle_t xHashEngineMutex = NULL;
static StaticSemaphore_t xHashEngineMutexStorage;

/*
 * Release defaults:
 *  - Buffered hardware mode is enabled by default.
 *  - Diagnostic self-test is disabled by default.
 *  - Diagnostic logs are disabled by default.
 *
 * For future HAL updates, these can be overridden at compile time.
 */
#ifndef STM32_SHA256_ALT_BUFFERED_HW_MODE_DEFAULT
    #define STM32_SHA256_ALT_BUFFERED_HW_MODE_DEFAULT    1
#endif

#ifndef STM32_SHA256_ALT_DIAG_SELFTEST
    #define STM32_SHA256_ALT_DIAG_SELFTEST               0
#endif

#if STM32_SHA256_ALT_DIAG_SELFTEST
static volatile uint8_t ucSha256SelfTestState = 0U; /* 0:not run, 1:running, 2:done */
static void prvRunSha256AltSelfTest( void );
#endif

static volatile uint8_t ucSha256ForceBufferedHwMode = ( uint8_t ) STM32_SHA256_ALT_BUFFERED_HW_MODE_DEFAULT;

static int prvHashEngineLock( void )
{
    if( xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED )
    {
        return 0;
    }

    if( xHashEngineMutex == NULL )
    {
        taskENTER_CRITICAL();
        if( xHashEngineMutex == NULL )
        {
            xHashEngineMutex = xSemaphoreCreateMutexStatic( &xHashEngineMutexStorage );
        }
        taskEXIT_CRITICAL();
    }

    if( xHashEngineMutex == NULL )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    if( xSemaphoreTake( xHashEngineMutex, portMAX_DELAY ) != pdTRUE )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    return 0;
}

static void prvHashEngineUnlock( void )
{
    if( ( xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED ) &&
        ( xHashEngineMutex != NULL ) )
    {
        ( void ) xSemaphoreGive( xHashEngineMutex );
    }
}

void mbedtls_sha256_init( mbedtls_sha256_context *ctx )
{
    memset( ctx, 0, sizeof( *ctx ) );
}

void mbedtls_sha256_free( mbedtls_sha256_context *ctx )
{
    if( ( ctx != NULL ) && ( ctx->hw_msg_buf != NULL ) )
    {
        vPortFree( ctx->hw_msg_buf );
        ctx->hw_msg_buf = NULL;
    }

    if( ctx != NULL )
    {
        ctx->hw_msg_len = 0U;
        ctx->hw_msg_cap = 0U;
    }
}

void mbedtls_sha256_clone( mbedtls_sha256_context *dst,
                           const mbedtls_sha256_context *src )
{
    uint8_t *pOldBuf = dst->hw_msg_buf;

    *dst = *src;

    if( pOldBuf != NULL )
    {
        vPortFree( pOldBuf );
    }

    if( src->hw_msg_buf != NULL )
    {
        dst->hw_msg_buf = pvPortMalloc( src->hw_msg_cap );
        if( dst->hw_msg_buf != NULL )
        {
            memcpy( dst->hw_msg_buf, src->hw_msg_buf, src->hw_msg_len );
        }
        else
        {
            dst->hw_msg_len = 0U;
            dst->hw_msg_cap = 0U;
        }
    }
}

static int hash_status_to_mbedtls( HAL_StatusTypeDef st )
{
    if( st == HAL_OK )
        return 0;

    return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
}

static int prvEnsureMsgCapacity( mbedtls_sha256_context *ctx, size_t needed )
{
    uint8_t *pNew;
    size_t newCap;

    if( needed <= ctx->hw_msg_cap )
    {
        return 0;
    }

    newCap = ( ctx->hw_msg_cap == 0U ) ? 256U : ctx->hw_msg_cap;
    while( newCap < needed )
    {
        if( newCap > ( ( size_t ) -1 / 2U ) )
        {
            return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
        }
        newCap *= 2U;
    }

    pNew = pvPortMalloc( newCap );
    if( pNew == NULL )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    if( ( ctx->hw_msg_buf != NULL ) && ( ctx->hw_msg_len > 0U ) )
    {
        memcpy( pNew, ctx->hw_msg_buf, ctx->hw_msg_len );
        vPortFree( ctx->hw_msg_buf );
    }

    ctx->hw_msg_buf = pNew;
    ctx->hw_msg_cap = newCap;

    return 0;
}

static int prvHashAccumulateBlock64( const uint8_t *pBlock64 )
{
    uint32_t ulAlignedBlock[ 16 ];
    HAL_StatusTypeDef st;

    /* Force 32-bit aligned source for HAL HASH input path. */
    memcpy( ulAlignedBlock, pBlock64, sizeof( ulAlignedBlock ) );
    st = HAL_HASH_Accumulate( &hhash,
                              ( const uint8_t * ) ulAlignedBlock,
                              64U,
                              HAL_MAX_DELAY );

    return hash_status_to_mbedtls( st );
}

static void prvSaveHalSavedStateToCtx( mbedtls_sha256_context *ctx )
{
    ctx->hw_init_saved = hhash.Init_saved;
    ctx->hw_in_ptr_saved = hhash.pHashInBuffPtr_saved;
    ctx->hw_out_ptr_saved = hhash.pHashOutBuffPtr_saved;
    ctx->hw_hash_in_count_saved = hhash.HashInCount_saved;
    ctx->hw_size_saved = hhash.Size_saved;
    ctx->hw_key_ptr_saved = hhash.pHashKeyBuffPtr_saved;
    ctx->hw_phase_saved = hhash.Phase_saved;
    ctx->hw_hal_saved_valid = 1U;
}

static int prvRestoreHalSavedStateFromCtx( const mbedtls_sha256_context *ctx )
{
    if( ctx->hw_hal_saved_valid == 0U )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    hhash.Init_saved = ctx->hw_init_saved;
    hhash.pHashInBuffPtr_saved = ctx->hw_in_ptr_saved;
    hhash.pHashOutBuffPtr_saved = ctx->hw_out_ptr_saved;
    hhash.HashInCount_saved = ctx->hw_hash_in_count_saved;
    hhash.Size_saved = ctx->hw_size_saved;
    hhash.pHashKeyBuffPtr_saved = ctx->hw_key_ptr_saved;
    hhash.Phase_saved = ctx->hw_phase_saved;

    return 0;
}

int mbedtls_sha256_starts( mbedtls_sha256_context *ctx, int is224 )
{
    int ret = 0;

    if( is224 != 0 )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

#if STM32_SHA256_ALT_DIAG_SELFTEST
    if( ucSha256SelfTestState == 0U )
    {
        ucSha256SelfTestState = 1U;
        prvRunSha256AltSelfTest();
        ucSha256SelfTestState = 2U;
    }
#endif

    memset( ctx->total, 0, sizeof( ctx->total ) );
    memset( ctx->buffer, 0, sizeof( ctx->buffer ) );
    ctx->is224 = 0;
    ctx->hw_snapshot_valid = 0;
    ctx->hw_hal_saved_valid = 0U;
    ctx->hw_msg_len = 0U;

    if( ucSha256ForceBufferedHwMode != 0U )
    {
        LogDebug( ( "SHA256 ALT: using buffered hardware mode." ) );
        ctx->hw_snapshot_valid = 1U;
        return 0;
    }

    ret = prvHashEngineLock();
    if( ret != 0 )
    {
        return ret;
    }

    /* Start a fresh SHA-256 hardware context and snapshot it into this SHA ctx. */
    HAL_StatusTypeDef st = HAL_HASH_DeInit( &hhash );
    if( st != HAL_OK )
    {
        ret = hash_status_to_mbedtls( st );
        goto cleanup;
    }

    st = HAL_HASH_Init( &hhash );
    if( st != HAL_OK )
    {
        ret = hash_status_to_mbedtls( st );
        goto cleanup;
    }

    HAL_HASH_Suspend( &hhash, ( uint8_t * ) ctx->hw_snapshot );
    ctx->hw_snapshot_valid = 1U;
    prvSaveHalSavedStateToCtx( ctx );

cleanup:
    prvHashEngineUnlock();
    return ret;
}

int mbedtls_sha256_update( mbedtls_sha256_context *ctx,
                           const unsigned char *input,
                           size_t ilen )
{
    int ret = 0;

    if( ilen == 0U )
    {
        return 0;
    }

    if( input == NULL )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    if( ucSha256ForceBufferedHwMode != 0U )
    {
        size_t needed = ctx->hw_msg_len + ilen;

        ret = prvEnsureMsgCapacity( ctx, needed );
        if( ret != 0 )
        {
            return ret;
        }

        memcpy( ctx->hw_msg_buf + ctx->hw_msg_len, input, ilen );
        ctx->hw_msg_len = needed;
        return 0;
    }

    if( ctx->hw_snapshot_valid == 0U )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    size_t fill = ctx->total[0] & 0x3F;   // bytes currently in buffer
    size_t left = 64 - fill;
    const unsigned char *p = input;
    size_t len = ilen;

    ctx->total[0] += len;
    if( ctx->total[0] < len )
    {
        ctx->total[1]++;
    }

    /* If no full 64-byte block can be formed, keep everything in SW buffer
     * and do not touch the HW HASH context. */
    if( ( fill != 0U ) && ( len < left ) )
    {
        memcpy( ctx->buffer + fill, p, len );
        return 0;
    }

    if( ( fill == 0U ) && ( len < 64U ) )
    {
        memcpy( ctx->buffer, p, len );
        return 0;
    }

    ret = prvHashEngineLock();
    if( ret != 0 )
    {
        return ret;
    }

    ret = prvRestoreHalSavedStateFromCtx( ctx );
    if( ret != 0 )
    {
        goto cleanup;
    }

    HAL_HASH_Resume( &hhash, ( uint8_t * ) ctx->hw_snapshot );

    if( ( fill != 0U ) && ( len >= left ) )
    {
        memcpy( ctx->buffer + fill, p, left );
        ret = prvHashAccumulateBlock64( ctx->buffer );
        if( ret != 0 )
        {
            goto cleanup;
        }

        p += left;
        len -= left;
        fill = 0U;
    }

    while( len >= 64U )
    {
        ret = prvHashAccumulateBlock64( p );
        if( ret != 0 )
        {
            goto cleanup;
        }

        p += 64U;
        len -= 64U;
    }

    if( len > 0U )
    {
        memcpy( ctx->buffer + fill, p, len );
    }

    HAL_HASH_Suspend( &hhash, ( uint8_t * ) ctx->hw_snapshot );
    ctx->hw_snapshot_valid = 1U;
    prvSaveHalSavedStateToCtx( ctx );

cleanup:
    prvHashEngineUnlock();
    return ret;
}

int mbedtls_sha256_finish( mbedtls_sha256_context *ctx,
                           unsigned char *output )
{
    int ret = 0;

    if( output == NULL )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    if( ucSha256ForceBufferedHwMode != 0U )
    {
        HAL_StatusTypeDef st;

        ret = prvHashEngineLock();
        if( ret != 0 )
        {
            return ret;
        }

        st = HAL_HASH_Start( &hhash,
                             ( const uint8_t * ) ctx->hw_msg_buf,
                             ( uint32_t ) ctx->hw_msg_len,
                             output,
                             HAL_MAX_DELAY );
        ret = hash_status_to_mbedtls( st );
        prvHashEngineUnlock();
        return ret;
    }

    if( ctx->hw_snapshot_valid == 0U )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    ret = prvHashEngineLock();
    if( ret != 0 )
    {
        return ret;
    }

    ret = prvRestoreHalSavedStateFromCtx( ctx );
    if( ret != 0 )
    {
        goto cleanup;
    }

    HAL_HASH_Resume( &hhash, ( uint8_t * ) ctx->hw_snapshot );

    size_t fill = ctx->total[0] & 0x3F;
    uint8_t digest[32];

    HAL_StatusTypeDef st = HAL_HASH_AccumulateLast(
        &hhash,
        fill ? ctx->buffer : NULL,
        ( uint32_t ) fill,
        digest,
        HAL_MAX_DELAY
    );

    ctx->hw_snapshot_valid = 0U;

    if( st != HAL_OK )
    {
        ret = MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
        goto cleanup;
    }

    if( ctx->is224 != 0 )
    {
        memcpy( output, digest, 28 );
    }
    else
    {
        memcpy( output, digest, sizeof( digest ) );
    }

    ret = 0;

cleanup:
    prvHashEngineUnlock();
    return ret;
}

/* One-shot helper used by md.c */
int mbedtls_sha256( const unsigned char *input,
                    size_t ilen,
                    unsigned char *output,
                    int is224 )
{
    if( ( input == NULL && ilen != 0U ) || ( output == NULL ) || ( is224 != 0 ) )
    {
        return MBEDTLS_ERR_SHA256_BAD_INPUT_DATA;
    }

    mbedtls_sha256_context ctx;
    int ret = 0;

    mbedtls_sha256_init( &ctx );

    ret = mbedtls_sha256_starts( &ctx, is224 );

    if( ret == 0 )
    {
      ret = mbedtls_sha256_update( &ctx, input, ilen );
    }

    if( ret == 0 )
    {
      ret = mbedtls_sha256_finish( &ctx, output );
    }

    mbedtls_sha256_free( &ctx );

    return ret;
}

#if STM32_SHA256_ALT_DIAG_SELFTEST
typedef struct
{
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    uint32_t datalen;
} sha256_ref_ctx_t;

static const uint32_t ulSha256RefK[64] =
{
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
};

static uint32_t prvRefRor( uint32_t x, uint32_t n )
{
    return ( x >> n ) | ( x << ( 32U - n ) );
}

static void prvSha256RefTransform( sha256_ref_ctx_t *ctx, const uint8_t block[64] )
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;
    uint32_t i;

    for( i = 0; i < 16U; i++ )
    {
        w[ i ] = ( ( uint32_t ) block[ i * 4U ] << 24 ) |
                 ( ( uint32_t ) block[ i * 4U + 1U ] << 16 ) |
                 ( ( uint32_t ) block[ i * 4U + 2U ] << 8 ) |
                 ( ( uint32_t ) block[ i * 4U + 3U ] );
    }

    for( i = 16U; i < 64U; i++ )
    {
        uint32_t s0 = prvRefRor( w[ i - 15U ], 7U ) ^ prvRefRor( w[ i - 15U ], 18U ) ^ ( w[ i - 15U ] >> 3U );
        uint32_t s1 = prvRefRor( w[ i - 2U ], 17U ) ^ prvRefRor( w[ i - 2U ], 19U ) ^ ( w[ i - 2U ] >> 10U );
        w[ i ] = w[ i - 16U ] + s0 + w[ i - 7U ] + s1;
    }

    a = ctx->state[ 0 ];
    b = ctx->state[ 1 ];
    c = ctx->state[ 2 ];
    d = ctx->state[ 3 ];
    e = ctx->state[ 4 ];
    f = ctx->state[ 5 ];
    g = ctx->state[ 6 ];
    h = ctx->state[ 7 ];

    for( i = 0U; i < 64U; i++ )
    {
        uint32_t s1 = prvRefRor( e, 6U ) ^ prvRefRor( e, 11U ) ^ prvRefRor( e, 25U );
        uint32_t ch = ( e & f ) ^ ( ( ~e ) & g );
        uint32_t s0 = prvRefRor( a, 2U ) ^ prvRefRor( a, 13U ) ^ prvRefRor( a, 22U );
        uint32_t maj = ( a & b ) ^ ( a & c ) ^ ( b & c );

        t1 = h + s1 + ch + ulSha256RefK[ i ] + w[ i ];
        t2 = s0 + maj;
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[ 0 ] += a;
    ctx->state[ 1 ] += b;
    ctx->state[ 2 ] += c;
    ctx->state[ 3 ] += d;
    ctx->state[ 4 ] += e;
    ctx->state[ 5 ] += f;
    ctx->state[ 6 ] += g;
    ctx->state[ 7 ] += h;
}

static void prvSha256RefInit( sha256_ref_ctx_t *ctx )
{
    memset( ctx, 0, sizeof( *ctx ) );
    ctx->state[ 0 ] = 0x6a09e667U;
    ctx->state[ 1 ] = 0xbb67ae85U;
    ctx->state[ 2 ] = 0x3c6ef372U;
    ctx->state[ 3 ] = 0xa54ff53aU;
    ctx->state[ 4 ] = 0x510e527fU;
    ctx->state[ 5 ] = 0x9b05688cU;
    ctx->state[ 6 ] = 0x1f83d9abU;
    ctx->state[ 7 ] = 0x5be0cd19U;
}

static void prvSha256RefUpdate( sha256_ref_ctx_t *ctx, const uint8_t *data, size_t len )
{
    size_t i;

    for( i = 0; i < len; i++ )
    {
        ctx->data[ ctx->datalen++ ] = data[ i ];
        if( ctx->datalen == 64U )
        {
            prvSha256RefTransform( ctx, ctx->data );
            ctx->bitlen += 512U;
            ctx->datalen = 0U;
        }
    }
}

static void prvSha256RefFinal( sha256_ref_ctx_t *ctx, uint8_t digest[32] )
{
    uint32_t i;
    uint32_t j;

    i = ctx->datalen;

    if( ctx->datalen < 56U )
    {
        ctx->data[ i++ ] = 0x80U;
        while( i < 56U )
        {
            ctx->data[ i++ ] = 0x00U;
        }
    }
    else
    {
        ctx->data[ i++ ] = 0x80U;
        while( i < 64U )
        {
            ctx->data[ i++ ] = 0x00U;
        }
        prvSha256RefTransform( ctx, ctx->data );
        memset( ctx->data, 0, 56U );
    }

    ctx->bitlen += ( uint64_t ) ctx->datalen * 8ULL;
    ctx->data[ 63 ] = ( uint8_t ) ( ctx->bitlen );
    ctx->data[ 62 ] = ( uint8_t ) ( ctx->bitlen >> 8U );
    ctx->data[ 61 ] = ( uint8_t ) ( ctx->bitlen >> 16U );
    ctx->data[ 60 ] = ( uint8_t ) ( ctx->bitlen >> 24U );
    ctx->data[ 59 ] = ( uint8_t ) ( ctx->bitlen >> 32U );
    ctx->data[ 58 ] = ( uint8_t ) ( ctx->bitlen >> 40U );
    ctx->data[ 57 ] = ( uint8_t ) ( ctx->bitlen >> 48U );
    ctx->data[ 56 ] = ( uint8_t ) ( ctx->bitlen >> 56U );
    prvSha256RefTransform( ctx, ctx->data );

    for( j = 0U; j < 8U; j++ )
    {
        digest[ j * 4U ] = ( uint8_t ) ( ctx->state[ j ] >> 24U );
        digest[ j * 4U + 1U ] = ( uint8_t ) ( ctx->state[ j ] >> 16U );
        digest[ j * 4U + 2U ] = ( uint8_t ) ( ctx->state[ j ] >> 8U );
        digest[ j * 4U + 3U ] = ( uint8_t ) ( ctx->state[ j ] );
    }
}

static int prvDigestEqual( const uint8_t a[32], const uint8_t b[32] )
{
    uint8_t d = 0U;
    size_t i;

    for( i = 0; i < 32U; i++ )
    {
        d |= ( uint8_t ) ( a[ i ] ^ b[ i ] );
    }

    return ( d == 0U ) ? 1 : 0;
}

static void prvLogDigestPrefix( const char *pcTag, const uint8_t hw[32], const uint8_t sw[32] )
{
    LogError( ( "SHA256 ALT diag mismatch [%s]: hw=%02x%02x%02x%02x sw=%02x%02x%02x%02x",
                pcTag,
                hw[ 0 ], hw[ 1 ], hw[ 2 ], hw[ 3 ],
                sw[ 0 ], sw[ 1 ], sw[ 2 ], sw[ 3 ] ) );
}

static void prvRunSha256AltSelfTest( void )
{
    static const uint8_t ucMsg1[] = "abc";
    static const uint8_t ucMsg2[] = "The quick brown fox jumps over the lazy dog";
    uint8_t ucPattern[257];
    mbedtls_sha256_context xHw;
    mbedtls_sha256_context xClone;
    sha256_ref_ctx_t xSw;
    uint8_t ucHw[32];
    uint8_t ucSw[32];
    uint8_t ucRef[32];
    size_t i;
    int rc;
    int iFailures = 0;

    for( i = 0; i < sizeof( ucPattern ); i++ )
    {
        ucPattern[ i ] = ( uint8_t ) i;
    }

    /* Case 1: one-shot short message. */
    mbedtls_sha256_init( &xHw );
    rc = mbedtls_sha256_starts( &xHw, 0 );
    rc |= mbedtls_sha256_update( &xHw, ucMsg1, sizeof( ucMsg1 ) - 1U );
    rc |= mbedtls_sha256_finish( &xHw, ucHw );
    mbedtls_sha256_free( &xHw );
    prvSha256RefInit( &xSw );
    prvSha256RefUpdate( &xSw, ucMsg1, sizeof( ucMsg1 ) - 1U );
    prvSha256RefFinal( &xSw, ucSw );
    if( ( rc != 0 ) || ( prvDigestEqual( ucHw, ucSw ) == 0 ) )
    {
        prvLogDigestPrefix( "case1-oneshot-abc", ucHw, ucSw );
        iFailures++;
    }

    /* Case 2: odd chunking across multiple blocks. */
    mbedtls_sha256_init( &xHw );
    rc = mbedtls_sha256_starts( &xHw, 0 );
    rc |= mbedtls_sha256_update( &xHw, ucPattern, 1U );
    rc |= mbedtls_sha256_update( &xHw, ucPattern + 1U, 63U );
    rc |= mbedtls_sha256_update( &xHw, ucPattern + 64U, 5U );
    rc |= mbedtls_sha256_update( &xHw, ucPattern + 69U, sizeof( ucPattern ) - 69U );
    rc |= mbedtls_sha256_finish( &xHw, ucHw );
    mbedtls_sha256_free( &xHw );
    prvSha256RefInit( &xSw );
    prvSha256RefUpdate( &xSw, ucPattern, sizeof( ucPattern ) );
    prvSha256RefFinal( &xSw, ucSw );
    if( ( rc != 0 ) || ( prvDigestEqual( ucHw, ucSw ) == 0 ) )
    {
        prvLogDigestPrefix( "case2-chunked-257", ucHw, ucSw );
        iFailures++;
    }

    /* Case 3: clone mid-stream and finish both copies. */
    mbedtls_sha256_init( &xHw );
    mbedtls_sha256_init( &xClone );
    rc = mbedtls_sha256_starts( &xHw, 0 );
    rc |= mbedtls_sha256_update( &xHw, ucMsg2, 17U );
    mbedtls_sha256_clone( &xClone, &xHw );
    rc |= mbedtls_sha256_update( &xHw, ucMsg2 + 17U, sizeof( ucMsg2 ) - 1U - 17U );
    rc |= mbedtls_sha256_update( &xClone, ucMsg2 + 17U, sizeof( ucMsg2 ) - 1U - 17U );
    rc |= mbedtls_sha256_finish( &xHw, ucHw );
    rc |= mbedtls_sha256_finish( &xClone, ucSw );
    mbedtls_sha256_free( &xHw );
    mbedtls_sha256_free( &xClone );
    prvSha256RefInit( &xSw );
    prvSha256RefUpdate( &xSw, ucMsg2, sizeof( ucMsg2 ) - 1U );
    prvSha256RefFinal( &xSw, ucRef );
    if( ( rc != 0 ) ||
        ( prvDigestEqual( ucHw, ucSw ) == 0 ) ||
        ( prvDigestEqual( ucHw, ucRef ) == 0 ) )
    {
        prvLogDigestPrefix( "case3-clone-hw-vs-clone", ucHw, ucSw );
        prvLogDigestPrefix( "case3-clone-hw-vs-ref", ucHw, ucRef );
        iFailures++;
    }

    if( iFailures == 0 )
    {
        LogInfo( ( "SHA256 ALT self-test passed (oneshot/chunked/clone)." ) );
    }
    else
    {
        LogError( ( "SHA256 ALT self-test failed (%d case(s)).", iFailures ) );
        ucSha256ForceBufferedHwMode = 1U;
        LogError( ( "SHA256 ALT: forcing buffered hardware mode." ) );
    }
}
#endif /* STM32_SHA256_ALT_DIAG_SELFTEST */
