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
