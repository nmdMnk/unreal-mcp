@echo off
REM ========================================
REM Spirrow-UnrealWise MCP Server Launcher
REM ========================================

echo Starting Unreal MCP Server...
echo.

REM Change to Python directory
cd /d "%~dp0Python"

REM Run the MCP server using uv
uv run unreal_mcp_server.py

REM If there's an error, pause to show the message
if errorlevel 1 (
    echo.
    echo Error occurred while starting the MCP server.
    pause
)
