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

    @mcp.tool()
    def list_gas_assets(
        ctx: Context,
        asset_type: str = "all",
        path_filter: Optional[str] = None
    ) -> Dict[str, Any]:
        """
        List GAS-related assets (GameplayEffects, GameplayAbilities, GameplayCues) in the project.

        Args:
            asset_type: Filter by asset type:
                - "effect": GameplayEffect Blueprints only
                - "ability": GameplayAbility Blueprints only
                - "cue": GameplayCue actors and notifies only
                - "attribute_set": AttributeSet classes only
                - "all": All GAS-related assets (default)
            path_filter: Filter by content path (e.g., "/Game/GAS/")

        Returns:
            Dict containing:
            - success: Whether the operation succeeded
            - effects: List of GameplayEffect assets
            - abilities: List of GameplayAbility assets
            - cues: List of GameplayCue assets
            - attribute_sets: List of AttributeSet classes
            - total_count: Total number of assets found

        Example:
            list_gas_assets(asset_type="effect", path_filter="/Game/GAS/Effects/")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "asset_type": asset_type,
                "path_filter": path_filter or ""
            }

            logger.info(f"Listing GAS assets (type={asset_type}, path={path_filter or 'all'})")
            response = unreal.send_command("list_gas_assets", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            total = response.get("total_count", 0)
            logger.info(f"Found {total} GAS assets")

            return response

        except Exception as e:
            error_msg = f"Error listing GAS assets: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def create_gameplay_effect(
        ctx: Context,
        name: str,
        duration_policy: str = "Instant",
        duration_magnitude: Optional[float] = None,
        period: Optional[float] = None,
        modifiers: Optional[List[Dict[str, Any]]] = None,
        application_tags: Optional[List[str]] = None,
        removal_tags: Optional[List[str]] = None,
        path: str = "/Game/GAS/Effects",
        rationale: Optional[str] = None
    ) -> Dict[str, Any]:
        """
        Create a GameplayEffect Blueprint asset.

        Args:
            name: Effect name (e.g., "GE_Damage", "GE_HealOverTime")
            duration_policy: Effect duration type:
                - "Instant": Applied once immediately (e.g., damage, heal)
                - "HasDuration": Lasts for a set time (e.g., buffs, debuffs)
                - "Infinite": Lasts until manually removed (e.g., passive effects)
            duration_magnitude: Duration in seconds (required if duration_policy is "HasDuration")
            period: Period in seconds for periodic effects (e.g., damage over time)
            modifiers: List of attribute modifiers, each containing:
                - attribute: Attribute name (e.g., "Health", "Mana", "Damage")
                - operation: "Add", "Multiply", "Divide", or "Override"
                - magnitude: Numeric value for the modifier
                Example: [{"attribute": "Health", "operation": "Add", "magnitude": -25.0}]
            application_tags: Gameplay tags to grant when effect is applied
            removal_tags: Gameplay tags to remove when effect is applied
            path: Content browser path for the asset
            rationale: Design rationale (auto-saved to knowledge base)

        Returns:
            Dict containing:
            - success: Whether the operation succeeded
            - asset_path: Path to the created asset
            - name: Name of the created effect

        Examples:
            # Instant damage
            create_gameplay_effect(
                name="GE_Damage",
                duration_policy="Instant",
                modifiers=[{"attribute": "Health", "operation": "Add", "magnitude": -25.0}]
            )

            # Heal over time
            create_gameplay_effect(
                name="GE_HealOverTime",
                duration_policy="HasDuration",
                duration_magnitude=5.0,
                period=1.0,
                modifiers=[{"attribute": "Health", "operation": "Add", "magnitude": 10.0}]
            )

            # Speed buff
            create_gameplay_effect(
                name="GE_SpeedBuff",
                duration_policy="HasDuration",
                duration_magnitude=10.0,
                modifiers=[{"attribute": "MoveSpeed", "operation": "Multiply", "magnitude": 1.5}],
                application_tags=["Status.Buff.Speed"]
            )
        """
        from unreal_mcp_server import get_unreal_connection
        from tools.rag_tools import record_rationale

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "name": name,
                "duration_policy": duration_policy,
                "duration_magnitude": duration_magnitude or 0.0,
                "period": period or 0.0,
                "modifiers": modifiers or [],
                "application_tags": application_tags or [],
                "removal_tags": removal_tags or [],
                "path": path
            }

            logger.info(f"Creating GameplayEffect: {name} ({duration_policy})")
            response = unreal.send_command("create_gameplay_effect", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            # Record rationale if provided and successful
            if rationale and response.get("success"):
                record_rationale(
                    action=f"create_gameplay_effect:{name}",
                    rationale=rationale,
                    details={
                        "duration_policy": duration_policy,
                        "modifiers": modifiers,
                        "path": path
                    }
                )

            if response.get("success"):
                logger.info(f"Created GameplayEffect: {response.get('asset_path', 'N/A')}")
            else:
                logger.error(f"Failed to create GameplayEffect: {response.get('error', 'Unknown error')}")

            return response

        except Exception as e:
            error_msg = f"Error creating gameplay effect: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
