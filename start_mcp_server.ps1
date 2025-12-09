# ========================================
# Spirrow-UnrealWise MCP Server Launcher
# ========================================

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Unreal MCP Server Launcher" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Get script directory
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# Load local configuration if it exists
Write-Host "[1/4] Loading configuration..." -ForegroundColor Yellow
$configLocalPath = Join-Path $scriptDir "config.local.ps1"
if (Test-Path $configLocalPath) {
    . $configLocalPath
    Write-Host "      Loaded: config.local.ps1" -ForegroundColor Gray
} else {
    Write-Host "      No config.local.ps1 found (using defaults)" -ForegroundColor Gray
    Write-Host "      Tip: Copy config.example.ps1 to config.local.ps1 to customize" -ForegroundColor DarkGray
    # Set default values
    if (-not $env:RAG_SERVER_URL) {
        $env:RAG_SERVER_URL = "http://localhost:8100"
    }
}
Write-Host "      RAG_SERVER_URL: $env:RAG_SERVER_URL" -ForegroundColor Gray
Write-Host "      OK" -ForegroundColor Green
Write-Host ""

# Change to Python directory
$pythonDir = Join-Path $scriptDir "Python"
Write-Host "[2/4] Changing to Python directory..." -ForegroundColor Yellow
Set-Location $pythonDir
Write-Host "      Directory: $pythonDir" -ForegroundColor Gray
Write-Host "      OK" -ForegroundColor Green
Write-Host ""

# Verify uv is installed
Write-Host "[3/4] Checking environment..." -ForegroundColor Yellow
try {
    $uvVersion = uv --version 2>&1
    Write-Host "      uv: $uvVersion" -ForegroundColor Gray
    Write-Host "      OK" -ForegroundColor Green
} catch {
    Write-Host "      ERROR: uv is not installed" -ForegroundColor Red
    Write-Host "      Please install uv: https://docs.astral.sh/uv/" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}
Write-Host ""

# Run the MCP server using uv
Write-Host "[4/4] Starting MCP Server..." -ForegroundColor Yellow
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  MCP SERVER IS NOW RUNNING" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Press Ctrl+C to stop the server" -ForegroundColor Cyan
Write-Host ""

try {
    uv run unreal_mcp_server.py
}
catch {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "  ERROR: MCP Server Failed" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    Read-Host "Press Enter to exit"
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Yellow
Write-Host "  MCP Server Stopped" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Yellow
