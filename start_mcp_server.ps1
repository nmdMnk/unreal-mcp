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
Write-Host "Note: If shutdown hangs, wait 5 seconds for automatic force kill" -ForegroundColor Yellow
Write-Host ""

# Create a job to run the server with timeout on shutdown
$serverJob = Start-Job -ScriptBlock {
    param($pythonDir)
    Set-Location $pythonDir
    & uv run unreal_mcp_server.py
} -ArgumentList $pythonDir

try {
    # Monitor the job and display output
    while ($serverJob.State -eq 'Running') {
        $jobOutput = Receive-Job -Job $serverJob
        if ($jobOutput) {
            Write-Host $jobOutput
        }
        Start-Sleep -Milliseconds 100
    }

    # Get final output
    $finalOutput = Receive-Job -Job $serverJob
    if ($finalOutput) {
        Write-Host $finalOutput
    }

    # Check if job failed
    if ($serverJob.State -eq 'Failed') {
        $error = $serverJob.ChildJobs[0].JobStateInfo.Reason
        throw $error
    }
}
catch {
    # Handle Ctrl+C or other interruptions
    Write-Host ""
    Write-Host "Shutdown signal received..." -ForegroundColor Yellow

    # Try to stop the job gracefully with 5 second timeout
    Write-Host "Attempting graceful shutdown (5 second timeout)..." -ForegroundColor Yellow
    Stop-Job -Job $serverJob

    $timeout = 5
    $elapsed = 0
    while ($serverJob.State -eq 'Running' -and $elapsed -lt $timeout) {
        Start-Sleep -Seconds 1
        $elapsed++
        Write-Host "." -NoNewline -ForegroundColor Gray
    }

    if ($serverJob.State -eq 'Running') {
        Write-Host ""
        Write-Host "Graceful shutdown timeout - force killing process..." -ForegroundColor Red

        # Force kill any uv or python processes related to our server
        Get-Process | Where-Object {
            $_.ProcessName -like "*uv*" -or
            ($_.ProcessName -like "*python*" -and $_.CommandLine -like "*unreal_mcp_server*")
        } | ForEach-Object {
            Write-Host "Killing process: $($_.ProcessName) (PID: $($_.Id))" -ForegroundColor Red
            Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue
        }
    } else {
        Write-Host ""
        Write-Host "Graceful shutdown completed" -ForegroundColor Green
    }

    # Output any remaining job output
    $remainingOutput = Receive-Job -Job $serverJob -ErrorAction SilentlyContinue
    if ($remainingOutput) {
        Write-Host $remainingOutput
    }
}
finally {
    # Always cleanup the job
    if ($serverJob) {
        Remove-Job -Job $serverJob -Force -ErrorAction SilentlyContinue
    }

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Yellow
    Write-Host "  MCP Server Stopped" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
}
