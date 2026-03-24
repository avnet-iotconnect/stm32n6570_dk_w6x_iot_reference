# STM32 SHA256 ALT + AWS TLS Debug Record (March 24, 2026)

## Purpose
This note records what failed, how it was debugged, the confirmed root cause, and the final fix for AWS MQTT/TLS connection failures when `MBEDTLS_SHA256_ALT` was enabled on STM32.

Use this document as a quick recovery guide if the issue appears again after HAL, mbedTLS, or project updates.

## Environment
- Target: STM32N6 + W6x Wi-Fi module
- TLS endpoint: `a1qwhobjtvew8t-ats.iot.us-west-1.amazonaws.com:8883`
- Date of debug session: **March 24, 2026**
- Relevant files:
  - `Appli/Core/Src/crypto/stm32_sha256_alt.c`
  - `Appli/Core/Src/crypto/sha256_alt.h`
  - `Appli/Core/Inc/mbedtls_config_hw.h`

## Initial Symptoms
- With hardware SHA256 ALT enabled:
  - TLS handshake repeatedly failed.
  - Typical error:
    - `x509_verify_cert() returned -9984 (-0x2700)`
    - `Certificate verification flags 00000008`
    - `Failed to perform TLS handshake`
- With software SHA256, TLS succeeded.
- AWS provisioning also showed earlier CSR constraint issues during one phase, but the main persistent runtime failure became TLS verification under HW SHA ALT.

## What We Verified First
1. COM port access and serial logging were stabilized (115200, 8N1).
2. Device root CA was corrected and re-imported (Amazon Root CA 1 / requested Amazon trust chain as needed during testing).
3. TLS debug level increased temporarily to capture handshake state transitions.
4. Handshake consistently reached certificate verification stage before failing.

Conclusion at this stage: this looked like a hash/signature verification path issue, not Wi-Fi or socket connectivity.

## Focused SHA256 Investigation
We instrumented SHA256 ALT with an internal diagnostic self-test that compared:
- HW SHA256 result vs software reference SHA256.
- Multiple patterns:
  - One-shot short message
  - Chunked odd-length message (257 bytes, uneven updates)
  - Clone mid-stream case

### Key Failing Signal
Consistent mismatch in chunked test:
- `case2-chunked-257`
- Example:
  - `hw=82cc988c`
  - `sw=54acfbfe`

This proved the hardware incremental path was producing a wrong digest for chunked update flow.

## Root Cause
The failure was in the **incremental HAL SHA flow** (suspend/resume + accumulate style path), not in one-shot hardware hashing.

Important detail:
- `HAL_HASH_Accumulate()` requires data sizes aligned to 4 bytes.
- Even with block handling logic, the incremental state/suspend-resume behavior in this platform path produced incorrect digest in chunked scenarios used by TLS transcript/certificate verification flows.

Impact:
- Wrong SHA256 digest during TLS verification steps.
- Certificate signature verification fails.
- TLS aborts with `-0x2700` and verification flag `0x00000008`.

## Final Resolution
Kept **hardware SHA256** but switched default behavior to **buffered hardware mode**:
- `update()` buffers message data.
- `finish()` performs one-shot `HAL_HASH_Start()` over full buffered message.

This avoids the problematic incremental HAL path while still using the hardware hash peripheral.

## Code Changes Made
### `Appli/Core/Src/crypto/stm32_sha256_alt.c`
- Added robust buffered hardware path.
- Kept incremental path in codebase for future reevaluation.
- Added compile-time feature gates for clean release behavior.

Release defaults now:
- `STM32_SHA256_ALT_BUFFERED_HW_MODE_DEFAULT = 1`
- `STM32_SHA256_ALT_DIAG_SELFTEST = 0`
- `STM32_SHA256_ALT_DIAG_LOGS = 0`

Meaning:
- External users get quiet logs and stable behavior.
- Engineers can re-enable diagnostics when needed.

### `Appli/Core/Src/crypto/sha256_alt.h`
- Context extended to hold buffered message storage and HAL state fields needed by the implementation.

## Validation Result (After Fix)
Observed logs showed:
- Successful TLS handshake:
  - `TLS handshake successful`
  - `Connection ... established`
- MQTT tasks started and published/subscribed successfully.
- End-to-end AWS IoT connection recovered with HW SHA ALT enabled.

## How To Re-enable Diagnostics Later
If future HAL/firmware updates claim fixes, re-enable diagnostics temporarily:

1. Compile with:
   - `STM32_SHA256_ALT_DIAG_SELFTEST=1`
   - `STM32_SHA256_ALT_DIAG_LOGS=1`
2. Optionally test incremental mode again with:
   - `STM32_SHA256_ALT_BUFFERED_HW_MODE_DEFAULT=0`
3. Run and check:
   - Self-test results (especially `case2-chunked-257`)
   - TLS handshake success/failure against AWS endpoint

Do not ship with diagnostics enabled unless needed.

## Fast Triage Checklist (If Problem Returns)
1. Confirm COM port is free and logs are captured.
2. Confirm correct root CA is on device.
3. Confirm whether build is buffered mode or incremental mode.
4. If incremental mode enabled, run self-test and check chunked case mismatch.
5. If any mismatch appears, force buffered hardware mode and retest TLS.
6. Verify AWS endpoint, cert, and time validity only after hash path is known-good.

## Practical Takeaway
This session confirmed:
- The issue was not general AWS connectivity.
- The issue was not solved by certificate replacement alone.
- The core bug was SHA256 ALT incremental behavior under chunked updates.
- Buffered one-shot hardware hashing is the stable production workaround/fix for this project baseline.

