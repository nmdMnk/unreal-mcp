# ========================================
# Spirrow-UnrealWise MCP Server Configuration (PowerShell)
# ========================================
#
# This is an example configuration file.
# To use it:
#   1. Copy this file to "config.local.ps1"
#   2. Edit config.local.ps1 with your settings
#   3. config.local.ps1 is ignored by git, so your settings stay private
#
# ========================================

# RAG Server URL
# Set this to the URL of your RAG knowledge server
# Default: http://localhost:8100
$env:RAG_SERVER_URL = "http://localhost:8100"

# You can add more environment variables here as needed
# Example:
# $env:UNREAL_ENGINE_PATH = "C:\Program Files\Epic Games\UE_5.3"
# $env:DEBUG_MODE = "true"
