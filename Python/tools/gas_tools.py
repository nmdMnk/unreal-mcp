"""
GAS (Gameplay Ability System) Tools for Unreal MCP.

This module provides tools for working with Unreal Engine's Gameplay Ability System.
"""

import logging
from typing import Dict, Any, List, Optional
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("SpirrowBridge")


def register_gas_tools(mcp: FastMCP):
    """Register GAS tools with the MCP server."""

    @mcp.tool()
    def add_gameplay_tags(
        ctx: Context,
        tags: List[str],
        comments: Optional[Dict[str, str]] = None
    ) -> Dict[str, Any]:
        """
        Add Gameplay Tags to DefaultGameplayTags.ini.

        Gameplay Tags are hierarchical labels used throughout the Gameplay Ability System
        for identifying abilities, effects, states, and more.

        Args:
            tags: List of tag strings in hierarchical format (e.g., ["Ability.Attack.Melee", "Status.Buff.Speed"])
            comments: Optional dict mapping tag to DevComment (e.g., {"Ability.Attack.Melee": "Basic melee attack"})

        Returns:
            Dict containing:
            - success: Whether the operation succeeded
            - added_tags: List of tags that were added
            - skipped_tags: List of tags that already existed
            - file_path: Path to the modified ini file

        Example:
            add_gameplay_tags(
                tags=["Ability.Attack.Melee", "Ability.Attack.Ranged", "Status.Debuff.Poison"],
                comments={"Ability.Attack.Melee": "Basic melee attack", "Status.Debuff.Poison": "Poison damage over time"}
            )
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "tags": tags,
                "comments": comments or {}
            }

            logger.info(f"Adding {len(tags)} gameplay tags")
            response = unreal.send_command("add_gameplay_tags", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            added_count = len(response.get("added_tags", []))
            skipped_count = len(response.get("skipped_tags", []))
            logger.info(f"Added {added_count} tags, skipped {skipped_count} existing tags")

            return response

        except Exception as e:
            error_msg = f"Error adding gameplay tags: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def list_gameplay_tags(
        ctx: Context,
        filter_prefix: Optional[str] = None
    ) -> Dict[str, Any]:
        """
        List all registered Gameplay Tags from DefaultGameplayTags.ini.

        Args:
            filter_prefix: Optional prefix to filter tags (e.g., "Ability" to get all Ability.* tags)

        Returns:
            Dict containing:
            - success: Whether the operation succeeded
            - tags: List of tag entries with tag and comment
            - count: Total number of tags

        Example:
            list_gameplay_tags(filter_prefix="Ability")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "filter_prefix": filter_prefix or ""
            }

            logger.info(f"Listing gameplay tags{' with prefix: ' + filter_prefix if filter_prefix else ''}")
            response = unreal.send_command("list_gameplay_tags", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            tag_count = response.get("count", 0)
            logger.info(f"Found {tag_count} gameplay tags")

            return response

        except Exception as e:
            error_msg = f"Error listing gameplay tags: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def remove_gameplay_tag(
        ctx: Context,
        tag: str
    ) -> Dict[str, Any]:
        """
        Remove a Gameplay Tag from DefaultGameplayTags.ini.

        Args:
            tag: The tag string to remove (e.g., "Ability.Attack.Melee")

        Returns:
            Dict containing:
            - success: Whether the operation succeeded
            - removed: Whether the tag was found and removed
            - message: Status message

        Example:
            remove_gameplay_tag(tag="Test.Tag.ToRemove")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "tag": tag
            }

            logger.info(f"Removing gameplay tag: {tag}")
            response = unreal.send_command("remove_gameplay_tag", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            if response.get("removed", False):
                logger.info(f"Successfully removed tag: {tag}")
            else:
                logger.info(f"Tag not found: {tag}")

            return response

        except Exception as e:
            error_msg = f"Error removing gameplay tag: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
