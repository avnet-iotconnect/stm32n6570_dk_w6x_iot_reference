#ifndef STM32_SHA256_ALT_H
#define STM32_SHA256_ALT_H

#include <stddef.h>
#include <stdint.h>
#include "stm32n6xx_hal_hash.h"

#define STM32_HASH_CSR_WORD_COUNT          ( 103U )
#define STM32_HASH_SNAPSHOT_WORD_COUNT     ( STM32_HASH_CSR_WORD_COUNT + 3U )

/*
 * Match the original mbedtls_sha256_context layout so that any
 * existing code that relies on sizeof/clone/etc. stays valid.
 *
 * We only actively use total[] and buffer[] in the HW offload,
 * but keeping state[] and is224 preserves ABI compatibility.
 */
typedef struct mbedtls_sha256_context
{
    uint32_t total[2];          /*!< The number of Bytes processed.  */
    uint32_t state[8];          /*!< The intermediate digest state.  */
    unsigned char buffer[64];   /*!< The data block being processed. */
    int is224;                  /*!< 0: SHA-256, 1: SHA-224.        */
    uint32_t hw_snapshot[STM32_HASH_SNAPSHOT_WORD_COUNT]; /*!< HASH peripheral snapshot. */
    uint8_t hw_snapshot_valid;  /*!< 1 when hw_snapshot contains valid context. */
    HASH_ConfigTypeDef hw_init_saved;
    const uint8_t * hw_in_ptr_saved;
    uint8_t * hw_out_ptr_saved;
    uint32_t hw_hash_in_count_saved;
    uint32_t hw_size_saved;
    uint8_t * hw_key_ptr_saved;
    HAL_HASH_PhaseTypeDef hw_phase_saved;
    uint8_t hw_hal_saved_valid;
    uint8_t * hw_msg_buf;
    size_t hw_msg_len;
    size_t hw_msg_cap;
}
mbedtls_sha256_context;

#endif /* STM32_SHA256_ALT_H */
