# Unreal MCP

Model Context Protocol (MCP) を使用して Unreal Engine 5.5+ と連携するための Python ブリッジです。

## セットアップ

1. Python 3.10 以上がインストールされていることを確認してください
2. `uv` をインストールします（まだの場合）:
   ```bash
   curl -LsSf https://astral.sh/uv/install.sh | sh
   ```
3. 仮想環境を作成して有効化します:
   ```bash
   uv venv
   source .venv/bin/activate  # Unix/macOS の場合
   # または
   .venv\Scripts\activate     # Windows の場合
   ```
4. 依存関係をインストールします:
   ```bash
   uv pip install -e .
   ```

5. （オプション）テスト用依存関係もインストール:
   ```bash
   uv pip install -e ".[test]"
   ```

この時点で、MCP クライアント（Claude Desktop、Cursor、Windsurf）を設定して Unreal MCP Server を使用できます。詳細は [MCP クライアントの設定](README.md#configuring-your-mcp-client) を参照してください。

## テストの実行

Unreal EditorでSpirrowBridgeプラグインが有効な状態で実行してください。

### スモークテスト（クイック確認）

```bash
python tests/smoke_test.py
```

### pytestによるテスト

```bash
# 全テスト
python tests/run_tests.py

# カテゴリ別
python tests/run_tests.py -m umg
python tests/run_tests.py -m blueprint

# 詳細出力
python tests/run_tests.py -v
```

詳細は [tests/README.md](tests/README.md) を参照してください。

## 設定

MCP サーバは `.env` ファイルまたは環境変数による設定をサポートしています。設定値は以下の優先順位で読み込まれます:

1. **環境変数**（最優先）
2. **`.env` ファイル**
3. **デフォルト値**（フォールバック）

### .env ファイルのセットアップ

1. サンプル設定ファイルをコピーします:
   ```bash
   cp .env.example .env
   ```

2. `.env` ファイルを編集して設定をカスタマイズします:
   ```bash
   # .env の記述例
   RAG_SERVER_URL=http://localhost:8100
   UNREAL_HOST=127.0.0.1
   UNREAL_PORT=55557
   ```

### 利用可能な設定オプション

| 変数名 | 説明 | デフォルト値 |
|--------|------|-------------|
| `RAG_SERVER_URL` | RAG ナレッジサーバの URL | `http://localhost:8100` |
| `UNREAL_HOST` | Unreal Engine 接続用ホストアドレス | `127.0.0.1` |
| `UNREAL_PORT` | Unreal Engine TCP 接続用ポート | `55557` |

### 環境変数の使用

設定値は環境変数として直接設定することもできます:

```bash
# Unix/macOS
export RAG_SERVER_URL=http://localhost:9999
python unreal_mcp_server.py

# Windows (PowerShell)
$env:RAG_SERVER_URL="http://localhost:9999"
python unreal_mcp_server.py

# Windows (CMD)
set RAG_SERVER_URL=http://localhost:9999
python unreal_mcp_server.py
```

**注意:** 環境変数は `.env` ファイルの設定よりも優先されます。

## テストスクリプト

[scripts](./scripts) フォルダにいくつかのテストスクリプトがあります。これらは MCP Server を起動せずに、直接接続でツールと Unreal Bridge をテストするのに便利です。

スクリプトを実行するには、依存関係がインストールされていること、または `uv` 仮想環境内で実行していることを確認してください。

## トラブルシューティング

- サーバを実行する前に、Unreal Engine エディタが起動して実行中であることを確認してください
- 詳細なエラー情報は `unreal_mcp.log` のログを確認してください

## 開発

新しいツールを追加するには、`SpirrowBridgeBridge.py` ファイルを変更して新しいコマンドハンドラを追加し、`unreal_mcp_server.py` ファイルを更新して HTTP API 経由で公開してください。
