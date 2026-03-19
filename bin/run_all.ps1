# Stop script if any command fails
$ErrorActionPreference = "Stop"

Write-Host "==============================="
Write-Host " Starting Flash + Config Flow"
Write-Host "==============================="

# Get current script directory
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$configPath = Join-Path $scriptDir "config.json"

# Resolve flash script path
$flashScript  = Join-Path $scriptDir "flash.ps1"
$logPath = Join-Path $scriptDir "log.txt"

# Verify scripts exist
if (-not (Test-Path $flashScript)) {
    Write-Error "flash.ps1 not found at $flashScript"
    exit 1
}

if (-not (Test-Path $configPath)) {
    Write-Error "config.json not found at $configPath"
    exit 1
}

# Read broker type to select provisioning flow
$config = Get-Content -Raw -Path $configPath | ConvertFrom-Json
$brokerType = $config.broker_type

# Select provision script based on broker_type
switch ($brokerType) {
    "mosquitto" { $provisionScript = Join-Path $scriptDir "provision_mosquitto.ps1" }
    "aws" { $provisionScript = Join-Path $scriptDir "provision_aws_single.ps1" }
    default {
        Write-Error "Unsupported broker_type '$brokerType'. Use 'mosquitto' or 'aws' in config.json."
        exit 1
    }
}

if (-not (Test-Path $provisionScript)) {
    Write-Error "Provision script not found at $provisionScript"
    exit 1
}

# Prompt user to set board to Dev mode before flashing
Write-Host ""
Write-Host "Board setup required:"
Write-Host "Set the STM32N6-DK board to Dev mode."
[void](Read-Host "Press Enter to continue")

# Run flash
Write-Host "`n--- Running Flash Script ---"
& $flashScript

if ($LASTEXITCODE -ne 0) {
    Write-Error "Flash failed. Aborting."
    exit $LASTEXITCODE
}

Write-Host "Flash completed successfully."

# Optional small delay
Start-Sleep -Seconds 2

# Prompt user to switch board mode before provisioning
Write-Host ""
Write-Host "Board setup required:"
Write-Host "Set the STM32N6-DK board to Flash mode, then power-cycle the board."
[void](Read-Host "Press Enter to continue")

# Run provision
Write-Host "`n--- Running Provision Script ($brokerType) ---"
# Show live output and append the same output to log.txt
& $provisionScript *>&1 | Tee-Object -FilePath $logPath -Append

if ($LASTEXITCODE -ne 0) {
    Write-Error "Provision failed."
    exit $LASTEXITCODE
}

Write-Host "`nAll steps completed successfully."
