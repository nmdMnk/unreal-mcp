@echo off
REM ========================================
REM Spirrow-UnrealWise MCP Server Launcher
REM ========================================

echo ========================================
echo   Unreal MCP Server Launcher
echo ========================================
echo.

REM Load local configuration if it exists
echo [1/4] Loading configuration...
if exist "%~dp0config.local.bat" (
    call "%~dp0config.local.bat"
    echo       Loaded: config.local.bat
) else (
    echo       No config.local.bat found (using defaults)
    echo       Tip: Copy config.example.bat to config.local.bat to customize
    REM Set default values if not already set
    if not defined RAG_SERVER_URL (
        set RAG_SERVER_URL=http://localhost:8100
    )
)
echo       RAG_SERVER_URL: %RAG_SERVER_URL%
echo       OK
echo.

REM Change to Python directory
echo [2/4] Changing to Python directory...
cd /d "%~dp0Python"
echo       Directory: %CD%
echo       OK
echo.

REM Verify uv is installed
echo [3/4] Checking environment...
uv --version >nul 2>&1
if errorlevel 1 (
    echo       ERROR: uv is not installed
    echo       Please install uv: https://docs.astral.sh/uv/
    pause
    exit /b 1
)
for /f "delims=" %%i in ('uv --version 2^>^&1') do set UV_VERSION=%%i
echo       uv: %UV_VERSION%
echo       OK
echo.

REM Run the MCP server using uv
echo [4/4] Starting MCP Server...
echo.
echo ========================================
echo   MCP SERVER IS NOW RUNNING
echo ========================================
echo.
echo Press Ctrl+C to stop the server
echo.

uv run unreal_mcp_server.py

REM If there's an error, pause to show the message
if errorlevel 1 (
    echo.
    echo ========================================
    echo   ERROR: MCP Server Failed
    echo ========================================
    echo.
    pause
) else (
    echo.
    echo ========================================
    echo   MCP Server Stopped
    echo ========================================
)
