# 開発用スクリプト

このリポジトリでは、ビルド・起動・チェック・テストを別々のスクリプトで実行します。

## 初回セットアップ

```bash
./setup.sh
```

Meson のビルドディレクトリを作成し、`developer_mode=true` を有効にします。Web Process Extension の変更をビルドディレクトリから読み込むため、この設定を有効にしておきます。

既存のビルドディレクトリがある場合は、同じスクリプトで再設定できます。

## よく使うコマンド

| コマンド | 内容 |
|---|---|
| `./build.sh` | コンパイルのみ実行 |
| `./run.sh` | ビルド済みの Epiphany を起動 |
| `./dev.sh` | コンパイル後に Epiphany を起動 |
| `./check.sh` | シェル構文、ShellCheck（導入済みの場合）、C/Blueprint整形、Git差分、Meson再設定、コンパイルを確認 |
| `./test.sh` | Meson のテストを実行 |

従来の「ビルドしてから起動する」操作は `./dev.sh` に移しました。`./build.sh` はコンパイル結果だけを確認できるようにしています。

## HACKING.md 準拠チェック

コードを変更したら、`HACKING.md` の規約に準拠しているか確認します。

### 自動チェック

`./check.sh` から `data/check-code-style` を実行し、CソースとBlueprintの整形を確認します。実行には `uncrustify` と `blueprint-compiler` が必要です。

検査対象は `embed/`、`lib/`、`src/`、`tests/` 以下の `.c` と `.blp` ファイルです。`contrib/` 以下と `.h` ファイルは対象外です。

### 手動チェック

整形チェックだけでは判定できないため、変更箇所について次の項目をレビューします。

- インデント、波括弧、1行の制御文の書き方。
- コールバック関数名の `_cb` 接尾辞。
- UTF-8であること、コメントが完全な文章になっていること。
- Cの型の選択、型キャストと変数名の間隔。
- `TRUE`、`FALSE`、`NULL` との明示的な比較を避けていること。
- クラス内シグナルの発行方法。
- `src/`、`embed/`、`lib/` のレイヤー構造。
- WebKitのWebコンテキストとWebプロセス拡張機能のIPC。
- Webコンテンツへ挿入するデータのエンコードとXSS対策。

自動チェックと手動チェックの両方で、`HACKING.md` の規約に違反していないことを確認します。

## ビルド設定

既定では `build/` を使用します。別のビルドディレクトリも指定できます。

```bash
BUILD_DIR=build-debug ./setup.sh
BUILD_DIR=build-debug ./build.sh
BUILD_DIR=build-debug ./run.sh
```

コンパイルは既定で1ジョブずつ実行し、エラーの出所を追いやすくしています。並列数を増やす場合は `JOBS` を指定します。

```bash
JOBS=4 ./build.sh
```

## ログ

ビルドとテストのログはビルドディレクトリ内に保存します。

- `build/logs/build.log`
- `build/logs/test.log`
- Meson のテスト詳細: `build/meson-logs/testlog.txt`

これらはビルド生成物として扱い、Git には登録しません。
