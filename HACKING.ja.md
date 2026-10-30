Epiphany の開発を始めるにあたって役立つ情報を紹介します。

# コードのビルド

Epiphany を正常にビルドするには、開発版の不安定な WebKit が必要になることがよくあります。
対処方法が分からない場合は、[開発 wiki ページ](https://welcome.gnome.org/app/Epiphany)を参照してください。

# コーディングスタイル

コードを読みやすく整った状態に保ち、パッチを受け入れてもらうために、以下の要件に従ってください。

 * インデントにはスペース2個を使用し、タブは使用しない。

 * 波括弧は K&R スタイルにする。

 * 1行で完結する制御文には波括弧を付けない。ただし、同じ条件分岐の連鎖に複数行の節が含まれる場合は除く。

 * コールバック関数の名前には `_cb` という接尾辞を付ける。

 * すべてのファイルを UTF-8 でエンコードする。

 * 型には `gchar` / `gint` / `gdouble` / … ではなく、`char` / `int` / `double` / … を使用する。

 * 実装ファイルでは、最初に `"config.h"`、次に対応する宣言ヘッダーをインクルードする。その後に空行を1行置き、ローカルヘッダーをアルファベット順に並べ、さらに空行を1行置いてから、システムヘッダーをアルファベット順に並べる。ヘッダーファイルも同じ規則に従うが、当然ながら `config.h` と自身のヘッダーに関する部分は除く。

 * コメントは完全な文にする。つまり、適切な大文字・小文字の使い分けと句読点を使用する。

 * 新しいファイルのインデントを正しく整えるための参考設定として、`data/kr-gnome-indent.cfg` が用意されている。このファイルは `uncrustify` 用の設定ファイルである。

 * 型キャストと変数名の間にはスペースを入れない。正しい例: `(int *)foo`。誤った例: `(int*) foo`。

 * `TRUE`、`FALSE`、`NULL` との明示的な比較は避ける。正しい例: `if (!condition)`、`if (!pointer)`、`if (integer == 0)`。誤った例: `if (condition == FALSE)`、`if (pointer == NULL)`、`if (!integer)`。

 * シグナルがクラスの一部である場合は `g_signal_emit ()` を使用する。それ以外の場合は `g_signal_emit_by_name ()` を使用する。

# コードの構造

## レイヤー構造

現在、コードは複数のレイヤーに分けて構成されています。上位レイヤーは下位レイヤーを全面的に利用できますが、下位レイヤーから上位レイヤーを利用することはできません。例外は、デリゲートオブジェクトを介する場合です。たとえば `EphyEmbedContainer` はデリゲートインターフェースの例です。これにより、`EphyWindow` が `src/` にあるにもかかわらず、`embed/` から `EphyWindow` の機能の一部を利用できます。レイヤーは次のとおりです。

 * `src/`: 最上位のレイヤー。主に GUI 関連のコードを置く。
 * `embed/`: Web ビューに関係するコードを置く。
 * `lib/`: 最下位のレイヤー。上位レイヤーの機能を必要としないヘルパークラスを置く。

ビルドシステムは、下位レイヤーのインクルードパスに上位レイヤーが含まれないよう制御しています。そのため、意図的に回避しようとしない限り、このレイヤー構造を破ることはできないはずです。

## GtkApplication と EphyShell

Epiphany には、シングルトンオブジェクトである `EphyShell` が1つあります。その継承階層は次のとおりです。

```
 - GApplication
 --- GtkApplication
 ----- EphyEmbedShell
 ------- EphyShell
```

`EphyShell` のインスタンスは厳密に1つだけ存在し、同時に `EphyEmbedShell` と `GtkApplication` でもあります。必要な型のポインターを取得するには、通常の GObject のキャストを使用してください。

`EphyShell` は、アプリケーション全体の状態を保持するシングルトンオブジェクトです。整理されたグローバル変数のようなものだと考えるとよいでしょう。`src/` 内のどこからでも `ephy_shell_get_default()` を使ってアクセスできます。

`EphyEmbedShell` は、レイヤー構造を保つために `EphyShell` とは別のクラスとして存在します。`embed/` と `src/` のどちらからでもアクセスできます。したがって、`embed/` からアクセスする必要があるグローバルな状態は、`EphyShell` ではなく `EphyEmbedShell` に置いてください。

## 重要な Epiphany オブジェクト

`EphyWindow` は `GtkApplicationWindow` のサブクラスであり、`GtkApplicationWindow` は `GtkWindow` のサブクラスです。つまり、これがウィンドウです。ウィンドウは同時にいくつでも開くことができます。`EphyWindow` は、(a) `GtkHeaderBar` のサブクラスである `EphyHeaderBar` と、(b) `HdyTabView` を含む `EphyTabView` を保持します。`EphyTabView` は1つ以上のタブを保持し、各タブは `EphyEmbed` です。重要なので繰り返します。1つの `EphyEmbed` が、ブラウザーの1つのタブに対応します。各 `EphyEmbed` は、`WebKitWebView` のサブクラスである `EphyWebView` を保持します。実際に Web ページを表示し、ブラウザーのさまざまな機能が動作するのは、このオブジェクトです。

## 重要な WebKitGTK オブジェクト

WebKitGTK は、WebKit を GTK の API から利用できるようにするラッパーを提供する WebKit のポートです。

WebKit は非常に便利です。JavaScript エンジン、HTML レイアウトエンジン、Web ページの実際の描画など、ブラウザーを構築する際の複雑さの95%を WebKit が引き受けます。Epiphany が扱う必要があるのは、残りの5%だけです。重要な WebKitGTK オブジェクトは次のとおりです。

 * `WebKitWebView`（`EphyWebView` の親クラス）。Web ページを表示する。
 * `WebKitWebContext`。Web ビュー間で共有される状態を管理するグローバルオブジェクト。

Epiphany は、ブラウザーの各タブに1つずつ `EphyWebView` を持ちます。一方、`EphyEmbedShell` が保持する `WebKitWebContext` は全体で厳密に1つです。`WebKitWebContext` を必要とする WebKit API を使用するときは、必ず `EphyEmbedShell` が保持する Web コンテキストを使用してください。**警告:** WebKit のデフォルトの `WebKitWebContext` を使用してはいけません。つまり、`WebKitWebContext *` 型の引数に `NULL` を渡してはいけません。また、`webkit_web_context_get_default()` も使用しないでください。

WebKitGTK には、[メイン API](https://webkitgtk.org/reference/webkit2gtk/unstable/index.html)、[DOM API](https://webkitgtk.org/reference/webkitdomgtk/unstable/index.html)、[JavaScriptCore API](https://webkitgtk.org/reference/jsc-glib/unstable/index.html)それぞれの個別ドキュメントがあります。

## 最新の WebKit のプロセスアーキテクチャ

現在の WebKit（旧 WebKit2）は、ブラウザーの堅牢性を高めるためにマルチプロセスアーキテクチャを採用しています。UI プロセス（Epiphany のメインプロセス）は、複数のサブプロセスを起動します。

 * Web コンテンツの描画を担当する、任意の数の `WebKitWebProcess`
 * ネットワーク要求やストレージなどを担当する、1つの `WebKitNetworkProcess`

WebKit は通常、ブラウザーのタブごとに個別の `WebKitWebProcess` を実行します。もっとも、これは厳密には正しくありません。タブが JavaScript を使って別のタブを作成する場合があります。そのような場合、Web ビュー同士は「関連付けられた」状態になり、同じ `WebKitWebProcess` を共有します。さらに、異なるドメイン間を移動するとき、WebKit はビューを別の Web プロセスに切り替えることがあります。

Epiphany は `GtkApplication` を使ってアプリケーションの一意性を保証するため、通常は UI プロセスが同時に1つだけ実行されます。例外は、シークレットモード、またはプライベートプロファイルモード（コマンドラインからのみ利用可能）を使用する場合です。この場合、メインの Epiphany プロセスとは状態が共有されません。

## Epiphany の Web プロセス拡張機能

Epiphany の一部の機能では、Web プロセス内でコードを実行する必要があります。このコードは「Web プロセス拡張機能」と呼ばれ、`embed/web-process-extension/` に置かれています。これは共有ライブラリ `libephywebprocessextension.so` にコンパイルされ、`$(pkglibdir)`（例: `/usr/lib64/epiphany`）にインストールされます。`EphyEmbedShell` は `webkit_web_context_set_web_process_extensions_directory()` を使って、WebKit にこの場所から拡張機能を読み込むよう指示します。これにより、Epiphany の UI プロセスと Web プロセス拡張機能は、`webkit_web_context_send_message_to_all_extensions()`、`webkit_web_view_send_message_to_page()`、`webkit_web_process_extension_send_message_to_context()`、`webkit_web_page_send_message_to_view()` などの WebKit IPC 関数を介して相互に通信できます。

Web プロセス拡張機能を変更する場合は、後述の開発者モードを有効にしてください。そうすることで Epiphany は、インストール済みの場所にある共有ライブラリではなく、ビルドディレクトリにある共有ライブラリを参照します。インストール済みの場所にあるものは、インストール済み Epiphany の Web プロセス拡張機能です。Web プロセス拡張機能やその JavaScript ファイルに加えた変更が反映されない場合は、開発者モードを有効にしていない可能性があります。

Epiphany は、追加の IPC 手段としてスクリプトメッセージハンドラーも使用します。これにより、Web 拡張機能から `JSCValue` を UI プロセスへ送り、`EphyEmbedShell` で受け取ることができます。

UI プロセス API の中心的なクラスが `WebKitWebContext` と `WebKitWebView` であるのに対応して、Web プロセス API には `WebKitWebProcessExtension` と `WebKitWebPage` があります。1つの `WebKitWebContext` は、1つ以上の `WebKitWebProcessExtension` を持つことができます。同様に、1つの `WebKitWebView` は、1つ以上の `WebKitWebPage` を持つことができます。ある時点でビュー内でアクティブになるページは1つだけです。それ以外のページは、プロセスの切り替えに備えたものです。

# セキュリティ

信頼できないデータを Web コンテンツに挿入するときは、XSS 脆弱性を防ぐため、挿入先のコンテキストに応じてデータを適切にエンコードする必要があります。たとえば、ページタイトル、URL、Web アプリの ID などは、いずれも悪意のある内容である可能性があります。[OWASP の XSS Prevention ルール](https://cheatsheetseries.owasp.org/cheatsheets/Cross_Site_Scripting_Prevention_Cheat_Sheet.html)を注意深く読み、理解してください。そうしなければ、問題を引き起こすことになります。`lib/ephy-output-encoding.h` には、この処理を支援する関数が用意されています。

JavaScript を扱う場合は、特にルール #8「DOM ベースの XSS を防ぐ」に注意してください。このルールは扱いが難しく、JavaScript コード全体を通して慎重に対応する必要があります。

# デバッグ

デバッグを有効にするには、ビルド設定オプション `-Ddeveloper_mode=true` を指定します。

## ログ

実行時には、ログサービスを有効にする必要があります。ログサービスを有効にするには、環境変数 `EPHY_LOG_MODULES` を設定します。形式は `<moduleName>[:<moduleName>]*` で、`moduleName` にはファイル名を指定します。たとえば `export EPHY_LOG_MODULES=ephy-window.c:ephy-autocompletion.c` のように設定します。特別なログモジュール名 `all` を指定すると、すべてのログモジュールが有効になります。

コード内にデバッグメッセージを出力するには、`LOG()` マクロを使用してください。

## 警告

実行時には、警告用のサービスを有効にする必要があります。警告をデバッグするには、環境変数 `EPHY_DEBUG_BREAK` を設定してください。

`EPHY_DEBUG_BREAK` に指定できる値は次のとおりです。

```
	stack		スタックトレースを出力する。

	suspend		警告が発生したときに実行を停止する。
                その後、デバッガーをプロセスに接続できる。

	trap		デバッガー内で Epiphany を実行するときに使用する。
                実行を停止し、デバッガーに制御を戻す。
```

## プロファイリング

実行時には、プロファイリングサービスを有効にする必要があります。有効にするには、環境変数 `EPHY_PROFILING_MODULES` を設定します。形式は `<moduleName>[:<moduleName>]*` で、`moduleName` にはファイル名を指定します。たとえば `export EPHY_PROFILE_MODULES=ephy-window.c:ephy-autocompletion.c` のように設定します。特別なプロファイリングモジュール名 `all` を指定すると、すべてのプロファイリングモジュールが有効になります。

コードの一部をプロファイリングするには、`START_PROFILER` マクロと `STOP_PROFILER` マクロを使用してください。
