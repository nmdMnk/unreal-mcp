# ========================================
# Spirrow-UnrealWise MCP Server Launcher
# ========================================

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Unreal MCP Server Launcher" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Change to Python directory
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$pythonDir = Join-Path $scriptDir "Python"
Write-Host "[1/3] Changing to Python directory..." -ForegroundColor Yellow
Set-Location $pythonDir
Write-Host "      Directory: $pythonDir" -ForegroundColor Gray
Write-Host "      OK" -ForegroundColor Green
Write-Host ""

# Optional: Set RAG server URL (uncomment and modify if needed)
Write-Host "[2/3] Setting up environment..." -ForegroundColor Yellow
# $env:RAG_SERVER_URL = "http://localhost:8100"
Write-Host "      RAG_SERVER_URL: $env:RAG_SERVER_URL" -ForegroundColor Gray
Write-Host "      OK" -ForegroundColor Green
Write-Host ""

# Run the MCP server using uv
Write-Host "[3/3] Starting MCP Server..." -ForegroundColor Yellow
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
