"""
Blueprint操作のテストスイート

Blueprint作成、コンポーネント追加、ノード操作のテスト
"""

import pytest
from test_framework import assert_success, assert_response_has


@pytest.mark.blueprint
class TestBlueprintCore:
    """BlueprintCoreCommands テスト"""
    
    def test_create_blueprint(self, test_suite, unique_name):
        """Blueprint作成テスト"""
        bp_name = unique_name("BP_Test")
        
        result = test_suite.run_command("create_blueprint", {
            "name": bp_name,
            "parent_class": "Actor",
            "path": "/Game/Test"
        })
        
        assert_success(result, "Blueprint作成")
        assert_response_has(result, "success", True)
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{bp_name}"
        })
    
    def test_compile_blueprint(self, test_suite, unique_name):
        """Blueprintコンパイルテスト"""
        bp_name = unique_name("BP_Compile")
        
        # Blueprint作成
        test_suite.run_command("create_blueprint", {
            "name": bp_name,
            "parent_class": "Actor",
            "path": "/Game/Test"
        })
        
        # コンパイル
        result = test_suite.run_command("compile_blueprint", {
            "blueprint_name": bp_name,
            "path": "/Game/Test"
        })
        
        assert_success(result, "Blueprintコンパイル")
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{bp_name}"
        })
    
    def test_duplicate_blueprint(self, test_suite, unique_name):
        """Blueprint複製テスト"""
        bp_name = unique_name("BP_Original")
        dup_name = unique_name("BP_Duplicate")
        
        # オリジナル作成
        test_suite.run_command("create_blueprint", {
            "name": bp_name,
            "parent_class": "Actor",
            "path": "/Game/Test"
        })
        
        # 複製
        result = test_suite.run_command("duplicate_blueprint", {
            "source_name": bp_name,
            "new_name": dup_name,
            "source_path": "/Game/Test",
            "destination_path": "/Game/Test"
        })
        
        assert_success(result, "Blueprint複製")
        
        test_suite.add_cleanup("delete_asset", {"asset_path": f"/Game/Test/{bp_name}"})
        test_suite.add_cleanup("delete_asset", {"asset_path": f"/Game/Test/{dup_name}"})


@pytest.mark.blueprint
class TestBlueprintComponent:
    """BlueprintComponentCommands テスト"""
    
    @pytest.fixture(autouse=True)
    def setup_blueprint(self, test_suite, unique_name):
        """各テスト前にBlueprintを作成"""
        self.bp_name = unique_name("BP_Component")
        test_suite.run_command("create_blueprint", {
            "name": self.bp_name,
            "parent_class": "Actor",
            "path": "/Game/Test"
        })
        yield
    
    def test_add_static_mesh_component(self, test_suite):
        """StaticMeshComponent追加テスト"""
        result = test_suite.run_command("add_component_to_blueprint", {
            "blueprint_name": self.bp_name,
            "component_type": "StaticMeshComponent",
            "component_name": "MeshComp",
            "path": "/Game/Test"
        })
        
        assert_success(result, "StaticMeshComponent追加")
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{self.bp_name}"
        })
    
    def test_set_static_mesh_properties(self, test_suite):
        """StaticMeshプロパティ設定テスト"""
        # コンポーネント追加
        test_suite.run_command("add_component_to_blueprint", {
            "blueprint_name": self.bp_name,
            "component_type": "StaticMeshComponent",
            "component_name": "CubeComp",
            "path": "/Game/Test"
        })
        
        # メッシュ設定
        result = test_suite.run_command("set_static_mesh_properties", {
            "blueprint_name": self.bp_name,
            "component_name": "CubeComp",
            "static_mesh": "/Engine/BasicShapes/Cube.Cube",
            "path": "/Game/Test"
        })
        
        assert_success(result, "StaticMesh設定")
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{self.bp_name}"
        })


@pytest.mark.node
class TestBlueprintNode:
    """BlueprintNodeCommands テスト"""
    
    @pytest.fixture(autouse=True)
    def setup_blueprint(self, test_suite, unique_name):
        """各テスト前にBlueprintを作成"""
        self.bp_name = unique_name("BP_Node")
        test_suite.run_command("create_blueprint", {
            "name": self.bp_name,
            "parent_class": "Actor",
            "path": "/Game/Test"
        })
        yield
    
    def test_add_event_node(self, test_suite):
        """イベントノード追加テスト"""
        result = test_suite.run_command("add_blueprint_event_node", {
            "blueprint_name": self.bp_name,
            "event_name": "ReceiveBeginPlay",
            "path": "/Game/Test"
        })
        
        assert_success(result, "BeginPlayノード追加")
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{self.bp_name}"
        })
    
    def test_add_print_string_node(self, test_suite):
        """PrintStringノード追加テスト"""
        result = test_suite.run_command("add_print_string_node", {
            "blueprint_name": self.bp_name,
            "message": "Hello Test",
            "path": "/Game/Test"
        })
        
        assert_success(result, "PrintStringノード追加")
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{self.bp_name}"
        })
    
    def test_add_delay_node(self, test_suite):
        """Delayノード追加テスト"""
        result = test_suite.run_command("add_delay_node", {
            "blueprint_name": self.bp_name,
            "duration": 1.0,
            "path": "/Game/Test"
        })
        
        assert_success(result, "Delayノード追加")
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{self.bp_name}"
        })
    
    def test_add_branch_node(self, test_suite):
        """Branchノード追加テスト"""
        result = test_suite.run_command("add_branch_node", {
            "blueprint_name": self.bp_name,
            "path": "/Game/Test"
        })
        
        assert_success(result, "Branchノード追加")
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{self.bp_name}"
        })
    
    def test_add_sequence_node(self, test_suite):
        """Sequenceノード追加テスト"""
        result = test_suite.run_command("add_sequence_node", {
            "blueprint_name": self.bp_name,
            "num_outputs": 3,
            "path": "/Game/Test"
        })
        
        assert_success(result, "Sequenceノード追加")
        
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{self.bp_name}"
        })


@pytest.mark.blueprint
@pytest.mark.integration
class TestBlueprintIntegration:
    """Blueprint統合テスト"""
    
    def test_create_complete_blueprint(self, test_suite, unique_name):
        """完全なBlueprintを作成する統合テスト"""
        bp_name = unique_name("BP_Complete")
        
        # 1. Blueprint作成
        result = test_suite.run_command("create_blueprint", {
            "name": bp_name,
            "parent_class": "Actor",
            "path": "/Game/Test"
        })
        assert_success(result, "Blueprint作成")
        
        # 2. StaticMeshComponent追加
        result = test_suite.run_command("add_component_to_blueprint", {
            "blueprint_name": bp_name,
            "component_type": "StaticMeshComponent",
            "component_name": "Mesh",
            "path": "/Game/Test"
        })
        assert_success(result, "Mesh追加")
        
        # 3. メッシュ設定
        result = test_suite.run_command("set_static_mesh_properties", {
            "blueprint_name": bp_name,
            "component_name": "Mesh",
            "static_mesh": "/Engine/BasicShapes/Cube.Cube",
            "path": "/Game/Test"
        })
        assert_success(result, "Mesh設定")
        
        # 4. BeginPlayイベント追加
        result = test_suite.run_command("add_blueprint_event_node", {
            "blueprint_name": bp_name,
            "event_name": "ReceiveBeginPlay",
            "path": "/Game/Test"
        })
        assert_success(result, "BeginPlay追加")
        
        # 5. PrintString追加
        result = test_suite.run_command("add_print_string_node", {
            "blueprint_name": bp_name,
            "message": "Blueprint Started!",
            "path": "/Game/Test"
        })
        assert_success(result, "PrintString追加")
        
        # 6. コンパイル
        result = test_suite.run_command("compile_blueprint", {
            "blueprint_name": bp_name,
            "path": "/Game/Test"
        })
        assert_success(result, "コンパイル")
        
        # クリーンアップ
        test_suite.add_cleanup("delete_asset", {
            "asset_path": f"/Game/Test/{bp_name}"
        })
        
        # サマリー出力
        summary = test_suite.get_summary()
        print(f"\n統合テスト結果: {summary['passed']}/{summary['total']} passed")
