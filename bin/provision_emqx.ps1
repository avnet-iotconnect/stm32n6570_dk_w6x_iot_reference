# Load configuration
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$configPath = Join-Path $scriptDir "config.json"
$config = Get-Content -Raw -Path $configPath | ConvertFrom-Json
$brokerType = $config.broker_type

# Guard: this script is emqx-only
if ($brokerType -ne "emqx") {
    throw "This script supports broker_type 'emqx' only. Current value: '$brokerType'."
}

# Auto-detect STLink Virtual COM Port
$portName = Get-CimInstance Win32_SerialPort | Where-Object { $_.Name -like "*STLink Virtual COM Port*" } | Select-Object -ExpandProperty DeviceID

if (-not $portName) {
    Write-Warning "STLink Virtual COM Port not found. Using fallback from config.json."
    $portName = $config.portName
}

$baudRate = 115200
$serialPort = $null

# EMQX connection + root CA settings
$defaultEmqxRootCaUrl = "https://cacerts.digicert.com/DigiCertGlobalRootG2.crt.pem"
$emqxRootCaUrl = $defaultEmqxRootCaUrl
if ($config.PSObject.Properties.Name -contains "emqx_root_ca_url" -and -not [string]::IsNullOrWhiteSpace($config.emqx_root_ca_url)) {
    $emqxRootCaUrl = $config.emqx_root_ca_url
}
$emqxRootCaPath = Join-Path $scriptDir "DigiCertGlobalRootG2.crt.pem"
$emqxEndpoint = "broker.emqx.io"
$emqxPort = 8883

function Send-Command {
    param (
        [string]$command,
        [int]$timeoutMs = 1500
    )

    $serialPort.WriteLine($command)

    $response = ""
    $sw = [System.Diagnostics.Stopwatch]::StartNew()

    while ($sw.ElapsedMilliseconds -lt $timeoutMs) {
        if ($serialPort.BytesToRead -gt 0) {
            $response += $serialPort.ReadExisting()
        }
        Start-Sleep -Milliseconds 50
    }

    return $response
}

# Send a full PEM/text file over the CLI serial session
function Send-FileContent {
    param ([string]$filePath)
    $content = Get-Content -Raw -Path $filePath
    $serialPort.WriteLine($content)
    Start-Sleep -Milliseconds 500
}

# Extract a PEM block from command response text
function Get-PemBlock {
    param(
        [string]$Text,
        [string]$BeginMarker,
        [string]$EndMarker
    )

    $pattern = [Regex]::Escape($BeginMarker) + "(.|\s)*?" + [Regex]::Escape($EndMarker)
    if ($Text -match $pattern) {
        return $matches[0].Trim()
    }

    return $null
}

# Download root CA file before importing to device
function Ensure-RootCaFile {
    param(
        [string]$Url,
        [string]$OutPath
    )

    Write-Host "Downloading root CA: $Url"
    try {
        Invoke-WebRequest -Uri $Url -OutFile $OutPath -UseBasicParsing
    }
    catch {
        throw "Failed to download root CA from '$Url': $($_.Exception.Message)"
    }
}

try {
    # Open serial session to board CLI
    $serialPort = New-Object System.IO.Ports.SerialPort $portName, $baudRate, "None", 8, "One"
    $serialPort.Open()

    # Wi-Fi settings
    Send-Command "conf set wifi_ssid "
    Send-Command "conf set wifi_credential "
    Send-Command "conf commit"
    Send-Command "reset"
    Start-Sleep -Seconds 5

    # Root CA
    Ensure-RootCaFile -Url $emqxRootCaUrl -OutPath $emqxRootCaPath
    Send-Command "pki import cert root_ca_cert"
    Send-FileContent $emqxRootCaPath

    # Key + self-signed cert generation
    Send-Command "pki generate key" 4000 | Out-Null
    $certResponse = Send-Command "pki generate cert" 6000
    $certPem = Get-PemBlock -Text $certResponse -BeginMarker "-----BEGIN CERTIFICATE-----" -EndMarker "-----END CERTIFICATE-----"

    if ($certPem) {
        $tlsCertPath = $null
        if ($config.PSObject.Properties.Name -contains "tls_cert" -and -not [string]::IsNullOrWhiteSpace($config.tls_cert)) {
            if ([System.IO.Path]::IsPathRooted($config.tls_cert)) {
                $tlsCertPath = $config.tls_cert
            } else {
                $tlsCertPath = Join-Path $scriptDir $config.tls_cert
            }
        } else {
            $tlsCertPath = Join-Path $scriptDir "client.crt"
        }

        $certPem | Out-File -Encoding ascii $tlsCertPath
        Write-Host "EMQX client certificate generated and saved to $tlsCertPath"
    } else {
        Write-Warning "Could not extract PEM certificate from 'pki generate cert' output. Continuing with device-stored certificate."
    }

    # MQTT config
    Send-Command "conf set mqtt_endpoint $emqxEndpoint"
    Send-Command "conf set mqtt_port $emqxPort"
    Send-Command "conf set wifi_ssid $($config.wifi_ssid)"
    Send-Command "conf set wifi_credential $($config.wifi_credential)"

    Send-Command "conf commit"
    Start-Sleep -Seconds 1
    Send-Command "reset"
}
finally {
    # Always close serial port, even on failure
    if ($null -ne $serialPort -and $serialPort.IsOpen) {
        $serialPort.Close()
        Write-Host "Serial port closed."
    }
}
