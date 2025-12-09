# ========================================
# Spirrow-UnrealWise MCP Server Launcher
# ========================================

Write-Host "Starting Unreal MCP Server..." -ForegroundColor Cyan
Write-Host ""

# Change to Python directory
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$pythonDir = Join-Path $scriptDir "Python"
Set-Location $pythonDir

# Optional: Set RAG server URL (uncomment and modify if needed)
# $env:RAG_SERVER_URL = "http://localhost:8100"

# Run the MCP server using uv
try {
    uv run unreal_mcp_server.py
}
catch {
    Write-Host ""
    Write-Host "Error occurred while starting the MCP server:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Read-Host "Press Enter to exit"
}
