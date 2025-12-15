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

    @mcp.tool()
    def create_gas_character(
        ctx: Context,
        name: str,
        parent_class: str = "Character",
        asc_component_name: str = "AbilitySystemComponent",
        replication_mode: str = "Mixed",
        default_abilities: Optional[List[str]] = None,
        default_effects: Optional[List[str]] = None,
        path: str = "/Game/GAS/Characters",
        rationale: Optional[str] = None
    ) -> Dict[str, Any]:
        """
        Create a GAS-enabled Character Blueprint with AbilitySystemComponent configured.

        This is a high-level tool that combines:
        - Blueprint creation
        - AbilitySystemComponent addition
        - Default abilities/effects configuration

        Args:
            name: Blueprint name (e.g., "BP_GASCharacter")
            parent_class: Parent class to inherit from (default: "Character")
            asc_component_name: Name for the AbilitySystemComponent (default: "AbilitySystemComponent")
            replication_mode: ASC replication mode for multiplayer:
                - "Full": Full replication (server authority)
                - "Mixed": Mixed mode (recommended for player-controlled)
                - "Minimal": Minimal replication (AI/NPCs)
            default_abilities: List of GameplayAbility class paths to grant by default
                Example: ["/Game/GAS/Abilities/GA_Attack.GA_Attack_C"]
            default_effects: List of GameplayEffect class paths to apply by default
                Example: ["/Game/GAS/Effects/GE_DefaultStats.GE_DefaultStats_C"]
            path: Content browser path for the Blueprint
            rationale: Design rationale (auto-saved to knowledge base)

        Returns:
            Dict containing:
            - success: Whether the operation succeeded
            - blueprint_path: Path to the created Blueprint
            - asc_component: Name of the ASC component
            - abilities_set: Number of default abilities configured
            - effects_set: Number of default effects configured

        Example:
            create_gas_character(
                name="BP_PlayerGAS",
                parent_class="Character",
                replication_mode="Mixed",
                default_abilities=[
                    "/Game/GAS/Abilities/GA_Jump.GA_Jump_C",
                    "/Game/GAS/Abilities/GA_Attack.GA_Attack_C"
                ],
                default_effects=[
                    "/Game/GAS/Effects/GE_DefaultStats.GE_DefaultStats_C"
                ],
                path="/Game/GAS/Characters"
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
                "parent_class": parent_class,
                "asc_component_name": asc_component_name,
                "replication_mode": replication_mode,
                "default_abilities": default_abilities or [],
                "default_effects": default_effects or [],
                "path": path
            }

            logger.info(f"Creating GAS Character: {name}")
            response = unreal.send_command("create_gas_character", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            # Record rationale if provided and successful
            if rationale and response.get("success"):
                record_rationale(
                    action=f"create_gas_character:{name}",
                    rationale=rationale,
                    details={
                        "parent_class": parent_class,
                        "replication_mode": replication_mode,
                        "abilities_count": len(default_abilities or []),
                        "effects_count": len(default_effects or []),
                        "path": path
                    }
                )

            if response.get("success"):
                logger.info(f"Created GAS Character: {response.get('blueprint_path', 'N/A')}")
            else:
                logger.error(f"Failed to create GAS Character: {response.get('error', 'Unknown error')}")

            return response

        except Exception as e:
            error_msg = f"Error creating GAS character: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def set_ability_system_defaults(
        ctx: Context,
        blueprint_name: str,
        asc_component_name: str = "AbilitySystemComponent",
        default_abilities: Optional[List[str]] = None,
        default_effects: Optional[List[str]] = None,
        path: str = "/Game/Blueprints"
    ) -> Dict[str, Any]:
        """
        Set default abilities and effects on an existing AbilitySystemComponent.

        Use this to configure an ASC that was already added to a Blueprint,
        either manually or via add_component_to_blueprint.

        Args:
            blueprint_name: Name of the Blueprint containing the ASC
            asc_component_name: Name of the AbilitySystemComponent in the Blueprint
            default_abilities: List of GameplayAbility class paths to grant
                Example: ["/Game/GAS/Abilities/GA_Attack.GA_Attack_C"]
            default_effects: List of GameplayEffect class paths to apply
                Example: ["/Game/GAS/Effects/GE_DefaultStats.GE_DefaultStats_C"]
            path: Content browser path where the Blueprint is located

        Returns:
            Dict containing:
            - success: Whether the operation succeeded
            - abilities_set: Number of abilities configured
            - effects_set: Number of effects configured

        Example:
            set_ability_system_defaults(
                blueprint_name="BP_ExistingCharacter",
                asc_component_name="AbilitySystem",
                default_abilities=["/Game/GAS/Abilities/GA_Dash.GA_Dash_C"],
                path="/Game/Characters"
            )
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "blueprint_name": blueprint_name,
                "asc_component_name": asc_component_name,
                "default_abilities": default_abilities or [],
                "default_effects": default_effects or [],
                "path": path
            }

            logger.info(f"Setting ASC defaults on: {blueprint_name}")
            response = unreal.send_command("set_ability_system_defaults", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            if response.get("success"):
                logger.info(f"Set {response.get('abilities_set', 0)} abilities, {response.get('effects_set', 0)} effects")
            else:
                logger.error(f"Failed to set ASC defaults: {response.get('error', 'Unknown error')}")

            return response

        except Exception as e:
            error_msg = f"Error setting ability system defaults: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def create_gameplay_ability(
        ctx: Context,
        name: str,
        parent_class: str = "GameplayAbility",
        ability_tags: Optional[List[str]] = None,
        cancel_abilities_with_tags: Optional[List[str]] = None,
        block_abilities_with_tags: Optional[List[str]] = None,
        activation_owned_tags: Optional[List[str]] = None,
        activation_required_tags: Optional[List[str]] = None,
        activation_blocked_tags: Optional[List[str]] = None,
        cost_effect: Optional[str] = None,
        cooldown_effect: Optional[str] = None,
        instancing_policy: str = "InstancedPerActor",
        net_execution_policy: str = "LocalPredicted",
        path: str = "/Game/GAS/Abilities",
        rationale: Optional[str] = None
    ) -> Dict[str, Any]:
        """
        Create a GameplayAbility Blueprint asset.

        Args:
            name: Ability name (e.g., "GA_Attack", "GA_Dash", "GA_FireBall")
            parent_class: Parent class (default: "GameplayAbility", can use custom C++ ability class)
            ability_tags: Tags that identify this ability (e.g., ["Ability.Attack.Melee"])
            cancel_abilities_with_tags: Tags of abilities to cancel when this activates
            block_abilities_with_tags: Tags of abilities blocked while this is active
            activation_owned_tags: Tags granted while ability is active
            activation_required_tags: Tags required to activate this ability
            activation_blocked_tags: Tags that prevent activation if present
            cost_effect: GameplayEffect path for ability cost (e.g., "/Game/GAS/Effects/GE_ManaCost.GE_ManaCost_C")
            cooldown_effect: GameplayEffect path for cooldown (e.g., "/Game/GAS/Effects/GE_Cooldown.GE_Cooldown_C")
            instancing_policy: How ability is instanced:
                - "NonInstanced": No instance created (most efficient, limited functionality)
                - "InstancedPerActor": One instance per actor (recommended)
                - "InstancedPerExecution": New instance each activation
            net_execution_policy: Network execution policy:
                - "LocalPredicted": Client predicts, server authoritative (recommended)
                - "LocalOnly": Client only, no replication
                - "ServerInitiated": Server starts, replicated to client
                - "ServerOnly": Server only
            path: Content browser path for the asset
            rationale: Design rationale (auto-saved to knowledge base)

        Returns:
            Dict containing:
            - success: Whether the operation succeeded
            - asset_path: Path to the created asset
            - name: Name of the created ability

        Examples:
            # Simple melee attack
            create_gameplay_ability(
                name="GA_MeleeAttack",
                ability_tags=["Ability.Attack.Melee"],
                activation_owned_tags=["State.Attacking"],
                block_abilities_with_tags=["Ability.Attack"],
                path="/Game/GAS/Abilities"
            )

            # Ability with cost and cooldown
            create_gameplay_ability(
                name="GA_FireBall",
                ability_tags=["Ability.Spell.Fire"],
                cost_effect="/Game/GAS/Effects/GE_ManaCost.GE_ManaCost_C",
                cooldown_effect="/Game/GAS/Effects/GE_SpellCooldown.GE_SpellCooldown_C",
                activation_blocked_tags=["State.Silenced"],
                path="/Game/GAS/Abilities"
            )

            # Dash ability
            create_gameplay_ability(
                name="GA_Dash",
                ability_tags=["Ability.Movement.Dash"],
                cancel_abilities_with_tags=["Ability.Attack"],
                activation_owned_tags=["State.Dashing"],
                cooldown_effect="/Game/GAS/Effects/GE_DashCooldown.GE_DashCooldown_C",
                instancing_policy="InstancedPerActor",
                net_execution_policy="LocalPredicted",
                path="/Game/GAS/Abilities"
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
                "parent_class": parent_class,
                "ability_tags": ability_tags or [],
                "cancel_abilities_with_tags": cancel_abilities_with_tags or [],
                "block_abilities_with_tags": block_abilities_with_tags or [],
                "activation_owned_tags": activation_owned_tags or [],
                "activation_required_tags": activation_required_tags or [],
                "activation_blocked_tags": activation_blocked_tags or [],
                "cost_effect": cost_effect or "",
                "cooldown_effect": cooldown_effect or "",
                "instancing_policy": instancing_policy,
                "net_execution_policy": net_execution_policy,
                "path": path
            }

            logger.info(f"Creating GameplayAbility: {name}")
            response = unreal.send_command("create_gameplay_ability", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            # Record rationale if provided and successful
            if rationale and response.get("success"):
                record_rationale(
                    action=f"create_gameplay_ability:{name}",
                    rationale=rationale,
                    details={
                        "parent_class": parent_class,
                        "ability_tags": ability_tags,
                        "instancing_policy": instancing_policy,
                        "path": path
                    }
                )

            if response.get("success"):
                logger.info(f"Created GameplayAbility: {response.get('asset_path', 'N/A')}")
            else:
                logger.error(f"Failed to create GameplayAbility: {response.get('error', 'Unknown error')}")

            return response

        except Exception as e:
            error_msg = f"Error creating gameplay ability: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
