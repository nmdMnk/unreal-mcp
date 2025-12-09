@echo off
REM ========================================
REM Spirrow-UnrealWise MCP Server Configuration (Batch)
REM ========================================
REM
REM This is an example configuration file.
REM To use it:
REM   1. Copy this file to "config.local.bat"
REM   2. Edit config.local.bat with your settings
REM   3. config.local.bat is ignored by git, so your settings stay private
REM
REM ========================================

REM RAG Server URL
REM Set this to the URL of your RAG knowledge server
REM Default: http://localhost:8100
set RAG_SERVER_URL=http://localhost:8100

REM You can add more environment variables here as needed
REM Example:
REM set UNREAL_ENGINE_PATH=C:\Program Files\Epic Games\UE_5.3
REM set DEBUG_MODE=true
