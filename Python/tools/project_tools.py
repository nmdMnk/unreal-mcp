"""
Project Tools for Unreal MCP.

This module provides tools for managing project-wide settings and configuration.
"""

import logging
from typing import Dict, Any
from mcp.server.fastmcp import FastMCP, Context

# Get logger
logger = logging.getLogger("SpirrowBridge")

def register_project_tools(mcp: FastMCP):
    """Register project tools with the MCP server."""
    
    @mcp.tool()
    def create_input_mapping(
        ctx: Context,
        action_name: str,
        key: str,
        input_type: str = "Action"
    ) -> Dict[str, Any]:
        """
        Create an input mapping for the project.
        
        Args:
            action_name: Name of the input action
            key: Key to bind (SpaceBar, LeftMouseButton, etc.)
            input_type: Type of input mapping (Action or Axis)
            
        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            params = {
                "action_name": action_name,
                "key": key,
                "input_type": input_type
            }
            
            logger.info(f"Creating input mapping '{action_name}' with key '{key}'")
            response = unreal.send_command("create_input_mapping", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Input mapping creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error creating input mapping: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def create_input_action(
        ctx: Context,
        action_name: str,
        value_type: str = "Digital",
        path: str = "/Game/Input"
    ) -> Dict[str, Any]:
        """
        Create an Enhanced Input Action asset.

        Args:
            action_name: Name of the input action (e.g., "IA_Jump")
            value_type: Type of input value - Digital, Axis1D, Axis2D, Axis3D
            path: Content browser path for the asset

        Returns:
            Dict containing the created asset path
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "action_name": action_name,
                "value_type": value_type,
                "path": path
            }

            logger.info(f"Creating input action '{action_name}' with value type '{value_type}' at '{path}'")
            response = unreal.send_command("create_input_action", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            logger.info(f"Input action creation response: {response}")
            return response

        except Exception as e:
            error_msg = f"Error creating input action: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def create_input_mapping_context(
        ctx: Context,
        context_name: str,
        path: str = "/Game/Input"
    ) -> Dict[str, Any]:
        """
        Create an Enhanced Input Mapping Context asset.

        Args:
            context_name: Name of the mapping context (e.g., "IMC_Default")
            path: Content browser path for the asset

        Returns:
            Dict containing the created asset path
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "context_name": context_name,
                "path": path
            }

            logger.info(f"Creating input mapping context '{context_name}' at '{path}'")
            response = unreal.send_command("create_input_mapping_context", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            logger.info(f"Input mapping context creation response: {response}")
            return response

        except Exception as e:
            error_msg = f"Error creating input mapping context: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def add_action_to_mapping_context(
        ctx: Context,
        context_name: str,
        action_name: str,
        key: str,
        trigger_type: str = "Down",
        modifiers: list = None,
        context_path: str = "/Game/Input",
        action_path: str = "/Game/Input"
    ) -> Dict[str, Any]:
        """
        Add an Input Action mapping to an Input Mapping Context.

        Args:
            context_name: Name of the mapping context
            action_name: Name of the input action to map
            key: Key to bind (SpaceBar, LeftShift, W, A, S, D, MouseX, MouseY, etc.)
            trigger_type: Trigger type - Down, Pressed, Released, Hold, HoldAndRelease
            modifiers: List of modifiers - Negate, SwizzleYXZ, DeadZone, etc.
            context_path: Path where the IMC asset is located
            action_path: Path where the IA asset is located

        Returns:
            Dict containing mapping result
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "context_name": context_name,
                "action_name": action_name,
                "key": key,
                "trigger_type": trigger_type,
                "modifiers": modifiers or [],
                "context_path": context_path,
                "action_path": action_path
            }

            logger.info(f"Adding action '{action_name}' to mapping context '{context_name}' with key '{key}'")
            response = unreal.send_command("add_action_to_mapping_context", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            logger.info(f"Action mapping response: {response}")
            return response

        except Exception as e:
            error_msg = f"Error adding action to mapping context: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    logger.info("Project tools registered successfully") 