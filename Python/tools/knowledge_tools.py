"""
Knowledge assistance tools for SpirrowUnrealWise.
Provides intelligent search across RAG knowledge and project classes.
"""

import logging
import re
from typing import Dict, Any, List, Optional
from mcp.server.fastmcp import Context

logger = logging.getLogger(__name__)

# UEの主要クラスとキーワードのマッピング
UE_CLASS_KEYWORDS = {
    # Movement
    "jump": ["Character", "CharacterMovementComponent", "Jump", "LaunchCharacter"],
    "move": ["CharacterMovementComponent", "MovementComponent", "NavMovementComponent"],
    "walk": ["CharacterMovementComponent", "MaxWalkSpeed"],
    "run": ["CharacterMovementComponent", "MaxWalkSpeed", "Sprint"],
    "sprint": ["CharacterMovementComponent", "MaxWalkSpeed"],
    "fly": ["CharacterMovementComponent", "Flying", "SetMovementMode"],
    "swim": ["CharacterMovementComponent", "Swimming", "SetMovementMode"],
    "crouch": ["CharacterMovementComponent", "Crouch", "UnCrouch"],

    # Combat
    "damage": ["ApplyDamage", "TakeDamage", "DamageType", "HealthComponent"],
    "health": ["HealthComponent", "TakeDamage", "Heal"],
    "attack": ["PlayAnimMontage", "AnimMontage", "MeleeAttack"],
    "shoot": ["SpawnActor", "ProjectileMovementComponent", "LineTrace"],
    "projectile": ["ProjectileMovementComponent", "SpawnActor"],
    "hit": ["LineTrace", "SweepTrace", "OnHit", "HitResult"],

    # AI
    "ai": ["AIController", "BehaviorTree", "BlackboardComponent", "BTTask"],
    "chase": ["AIMoveTo", "MoveToActor", "MoveToLocation"],
    "patrol": ["AIMoveTo", "Waypoint", "SplineComponent"],
    "follow": ["AIMoveTo", "MoveToActor"],
    "enemy": ["AIController", "BehaviorTree", "Pawn"],

    # Physics
    "physics": ["PrimitiveComponent", "SimulatePhysics", "AddForce", "AddImpulse"],
    "collision": ["CollisionComponent", "OnComponentBeginOverlap", "OnComponentHit"],
    "overlap": ["OnComponentBeginOverlap", "OnComponentEndOverlap"],
    "trigger": ["BoxComponent", "SphereComponent", "OnComponentBeginOverlap"],

    # Animation
    "animation": ["AnimInstance", "PlayAnimMontage", "AnimBlueprint"],
    "anim": ["AnimInstance", "PlayAnimMontage", "AnimBlueprint"],
    "montage": ["PlayAnimMontage", "AnimMontage", "OnMontageEnded"],

    # Input
    "input": ["EnhancedInputComponent", "InputAction", "InputMappingContext"],
    "key": ["InputAction", "EnhancedInputComponent", "BindAction"],
    "controller": ["PlayerController", "AIController", "GetController"],

    # UI
    "ui": ["UserWidget", "CreateWidget", "AddToViewport"],
    "widget": ["UserWidget", "CreateWidget", "AddToViewport"],
    "hud": ["HUD", "UserWidget", "DrawText"],

    # Audio
    "sound": ["AudioComponent", "PlaySound", "USoundBase"],
    "music": ["AudioComponent", "PlaySound", "FadeIn", "FadeOut"],

    # Spawn
    "spawn": ["SpawnActor", "SpawnActorDeferred", "BeginDeferredActorSpawnFromClass"],
    "create": ["SpawnActor", "NewObject", "CreateDefaultSubobject"],
    "destroy": ["DestroyActor", "DestroyComponent", "SetLifeSpan"],

    # Camera
    "camera": ["CameraComponent", "SpringArmComponent", "SetViewTarget"],
    "view": ["CameraComponent", "SetViewTarget", "PlayerCameraManager"],

    # Save/Load
    "save": ["SaveGame", "SaveGameToSlot", "AsyncSaveGameToSlot"],
    "load": ["LoadGameFromSlot", "AsyncLoadGameFromSlot"],

    # Timer
    "timer": ["SetTimer", "ClearTimer", "GetWorldTimerManager"],
    "delay": ["Delay", "SetTimer", "RetriggerableDelay"],

    # Networking
    "replicate": ["Replicated", "ReplicatedUsing", "ServerRPC", "ClientRPC"],
    "network": ["Replicated", "NetMulticast", "HasAuthority"],
}

# 日本語キーワードマッピング
JP_KEYWORD_MAP = {
    "ジャンプ": "jump",
    "移動": "move",
    "歩く": "walk",
    "走る": "run",
    "スプリント": "sprint",
    "飛ぶ": "fly",
    "泳ぐ": "swim",
    "しゃがむ": "crouch",
    "ダメージ": "damage",
    "体力": "health",
    "攻撃": "attack",
    "撃つ": "shoot",
    "弾": "projectile",
    "当たり": "hit",
    "敵": "enemy",
    "追いかける": "chase",
    "巡回": "patrol",
    "ついていく": "follow",
    "物理": "physics",
    "衝突": "collision",
    "重なり": "overlap",
    "トリガー": "trigger",
    "アニメ": "animation",
    "アニメーション": "animation",
    "入力": "input",
    "キー": "key",
    "UI": "ui",
    "ウィジェット": "widget",
    "音": "sound",
    "音楽": "music",
    "生成": "spawn",
    "作成": "create",
    "削除": "destroy",
    "破壊": "destroy",
    "カメラ": "camera",
    "視点": "view",
    "保存": "save",
    "読み込み": "load",
    "タイマー": "timer",
    "遅延": "delay",
    "ネットワーク": "network",
    "同期": "replicate",
}


def extract_keywords(query: str) -> List[str]:
    """
    クエリから関連キーワードを抽出する
    """
    keywords = []
    query_lower = query.lower()

    # 日本語キーワードを英語に変換
    for jp, en in JP_KEYWORD_MAP.items():
        if jp in query:
            keywords.append(en)

    # 英語キーワードを直接検索
    for en_key in UE_CLASS_KEYWORDS.keys():
        if en_key in query_lower:
            keywords.append(en_key)

    return list(set(keywords))


def get_suggested_classes(keywords: List[str]) -> List[Dict[str, str]]:
    """
    キーワードから推奨クラスを取得
    """
    suggestions = []
    seen = set()

    for keyword in keywords:
        if keyword in UE_CLASS_KEYWORDS:
            for class_name in UE_CLASS_KEYWORDS[keyword]:
                if class_name not in seen:
                    seen.add(class_name)
                    suggestions.append({
                        "name": class_name,
                        "type": "engine",
                        "keyword": keyword
                    })

    return suggestions


def register_knowledge_tools(mcp):
    """Register knowledge assistance tools with the MCP server."""

    @mcp.tool()
    async def find_relevant_nodes(
        ctx: Context,
        query: str,
        include_rag: bool = True,
        include_project: bool = True,
        max_rag_results: int = 5,
        max_project_results: int = 10
    ) -> Dict[str, Any]:
        """
        Find relevant nodes, classes, and assets based on what you want to accomplish.

        This tool combines RAG knowledge search with project class scanning to provide
        comprehensive suggestions for implementing game features.

        Args:
            query: What you want to accomplish (e.g., "implement jumping", "make enemy chase player")
            include_rag: Whether to search RAG knowledge base
            include_project: Whether to search project classes
            max_rag_results: Maximum number of RAG results
            max_project_results: Maximum number of project class results

        Returns:
            Dict containing:
            - query: Original query
            - keywords: Extracted keywords
            - rag_results: Relevant knowledge from RAG (if include_rag=True)
            - suggested_classes: Engine classes/functions that might help
            - project_matches: Relevant project classes and blueprints

        Examples:
            # Find how to implement jumping
            find_relevant_nodes("ジャンプを実装したい")

            # Find AI-related classes for enemy behavior
            find_relevant_nodes("make enemy chase the player")

            # Search only project classes
            find_relevant_nodes("character movement", include_rag=False)
        """
        from unreal_mcp_server import get_unreal_connection

        result = {
            "query": query,
            "keywords": [],
            "rag_results": [],
            "suggested_classes": [],
            "project_matches": []
        }

        try:
            # 1. キーワード抽出
            keywords = extract_keywords(query)
            result["keywords"] = keywords
            logger.info(f"Extracted keywords: {keywords}")

            # 2. 推奨クラスを取得
            suggested = get_suggested_classes(keywords)
            result["suggested_classes"] = suggested

            # 3. RAG検索
            if include_rag:
                try:
                    # RAGサーバーに直接接続
                    import httpx
                    rag_url = "http://localhost:8100"

                    async with httpx.AsyncClient() as client:
                        response = await client.post(
                            f"{rag_url}/search",
                            json={
                                "query": query,
                                "n_results": max_rag_results
                            },
                            timeout=10.0
                        )

                        if response.status_code == 200:
                            rag_data = response.json()
                            if rag_data.get("results"):
                                for i, doc in enumerate(rag_data["results"].get("documents", [[]])[0]):
                                    metadata = {}
                                    if rag_data["results"].get("metadatas"):
                                        metadata = rag_data["results"]["metadatas"][0][i] if i < len(rag_data["results"]["metadatas"][0]) else {}

                                    distance = 0.0
                                    if rag_data["results"].get("distances"):
                                        distance = rag_data["results"]["distances"][0][i] if i < len(rag_data["results"]["distances"][0]) else 0.0

                                    result["rag_results"].append({
                                        "content": doc,
                                        "category": metadata.get("category", "unknown"),
                                        "relevance": round(1.0 - distance, 2) if distance else 0.0
                                    })
                except Exception as e:
                    logger.warning(f"RAG search failed: {e}")

            # 4. プロジェクトクラス検索
            if include_project:
                try:
                    unreal = get_unreal_connection()
                    if unreal:
                        # キーワードに基づいて親クラスフィルタを決定
                        parent_filters = []
                        for kw in keywords:
                            if kw in ["jump", "move", "walk", "run", "sprint", "crouch"]:
                                parent_filters.append("Character")
                            elif kw in ["ai", "chase", "patrol", "follow", "enemy"]:
                                parent_filters.extend(["AIController", "Character"])
                            elif kw in ["ui", "widget", "hud"]:
                                parent_filters.append("UserWidget")
                            elif kw in ["animation", "anim", "montage"]:
                                parent_filters.append("AnimInstance")

                        parent_filters = list(set(parent_filters))

                        # 各フィルタでスキャン
                        all_matches = []
                        seen_paths = set()

                        if parent_filters:
                            for parent in parent_filters[:3]:  # 最大3つの親クラスで検索
                                response = unreal.send_command("scan_project_classes", {
                                    "parent_class": parent,
                                    "exclude_reinst": True
                                })

                                if response:
                                    # C++クラス
                                    for cls in response.get("cpp_classes", [])[:max_project_results]:
                                        if cls["path"] not in seen_paths:
                                            seen_paths.add(cls["path"])
                                            all_matches.append({
                                                "name": cls["name"],
                                                "path": cls["path"],
                                                "type": "cpp",
                                                "parent": cls["parent"],
                                                "module": cls.get("module", "")
                                            })

                                    # Blueprint
                                    for bp in response.get("blueprints", [])[:max_project_results]:
                                        if bp["path"] not in seen_paths:
                                            seen_paths.add(bp["path"])
                                            all_matches.append({
                                                "name": bp["name"],
                                                "path": bp["path"],
                                                "type": "blueprint",
                                                "parent": bp["parent"]
                                            })
                        else:
                            # フィルタなしでプロジェクトモジュールのみスキャン
                            response = unreal.send_command("scan_project_classes", {
                                "class_type": "cpp",
                                "module_filter": "TrapxTrapCpp",  # TODO: 動的に取得
                                "exclude_reinst": True
                            })

                            if response:
                                for cls in response.get("cpp_classes", [])[:max_project_results]:
                                    all_matches.append({
                                        "name": cls["name"],
                                        "path": cls["path"],
                                        "type": "cpp",
                                        "parent": cls["parent"],
                                        "module": cls.get("module", "")
                                    })

                        result["project_matches"] = all_matches[:max_project_results]

                except Exception as e:
                    logger.warning(f"Project class search failed: {e}")

            return result

        except Exception as e:
            error_msg = f"Error in find_relevant_nodes: {e}"
            logger.error(error_msg)
            return {"error": error_msg, **result}
