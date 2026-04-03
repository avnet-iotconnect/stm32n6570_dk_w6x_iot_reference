# Troubleshooting

Common issues and practical fixes.

## COM Port Busy / Access Denied

Symptoms:

- Serial port open fails
- `Access to the port 'COMxx' is denied`

Fix:

1. Close other serial tools using the same COM port (PuTTY, TeraTerm, VS Code serial monitor, etc.).
2. Rerun `bin\run_all.ps1` or the provisioning script.

## Board Does Not Boot or Debug

Check:

1. Board mode is correct for the current step (Dev mode for debug/flash sequence).
2. If board mode changed mid-session, reflash and retry.

## Signed Application Does Not Boot

Symptoms:

- flash appears to succeed
- FSBL starts, but the signed application does not boot correctly
- behavior changes depending on STM32CubeProgrammer version

Check:

1. If you built from STM32CubeIDE, run `bin\copy_hex_from_project.ps1` before `bin\run_all.ps1` or `bin\flash.ps1`.
2. Confirm the staged files under `bin\Appli\...` and `bin\FSBL\...` match the build configuration you actually built.
3. For this repo's legacy STM32N6 signing command, STM32CubeProgrammer `2.20.x` is known-good.
4. STM32CubeProgrammer `2.21.0+` changed STM32N6 signing behavior and requires `-align` / `--align` for STM32N6 images.

References:

- ST community note on STM32N6 signing behavior in STM32CubeProgrammer v2.21.0:
  - https://community.st.com/t5/stm32cubeprogrammer-mcus/signingtool-for-stm32n6-in-stm32cubeprogrammer-v2-21-0/td-p/859154
- ST community report that the STM32N6 command worked in 2.20 and changed in 2.21:
  - https://community.st.com/t5/stm32cubeprogrammer-mcus/no-padding-align-with-padding-in-stm32-signingtool-cli-2-21/td-p/859110

## Debug Load Issues After STM32CubeMX Regeneration

STM32CubeMX can overwrite boot files used by the debug flow.

Verify and re-apply in `Middlewares/ST/STM32_ExtMem_Manager/boot/stm32_boot_lrun.c`:

```c
#if !defined(_DEBUG_)
    retr = CopyApplication();
#endif
```

Then run `update.sh` and restart debug.

## AWS Provisioning Failures

Check:

1. `aws configure` is valid for the intended account/region.
2. IAM permissions allow IoT thing/certificate/policy operations.
3. Retry with `bin\provision_aws_single.ps1`.
