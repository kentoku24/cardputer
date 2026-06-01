# cardputer — Claude Code 承認用 BLEキーボード

M5Stack Cardputer (ESP32-S3) を **BLEキーボード**としてPC/Macにペアリングし、
Claude Code の権限プロンプトに対して **3操作だけ**を送出する専用デバイスにします。

| Cardputerキー | 操作 | 送出 | Claude Code 上の意味 |
|:---:|:---|:---:|:---|
| `1` | 承認 | `1` | Yes |
| `2` | 常に承認 | `2` | Yes, and don't ask again |
| `3` | 不承認 | `3` | No |

Claude Code の TUI は数字キーで選択肢を即決定できるため、数字を1打鍵送るだけで確定します。

## 必要なもの

- M5Stack Cardputer (ESP32-S3 / StampS3)
- [PlatformIO](https://platformio.org/)（VSCode拡張 もしくは `pip install platformio`）

## ビルド & 書き込み

```bash
# 依存ライブラリの取得 + ビルド
pio run

# Cardputer をUSB接続して書き込み
pio run -t upload

# シリアルログ確認（任意）
pio device monitor
```

## 使い方

1. 書き込み後、Cardputer 画面に「Claude Code 承認」と3つのボタンが表示される。
2. PC/Mac の Bluetooth 設定で **`Cardputer Approver`** を探してペアリングする。
3. 画面右上が「● 接続中」になれば準備完了。
4. Claude Code の権限プロンプトが出たら、Cardputer の `1` / `2` / `3` キーを押す。
   - 押すと該当行が白く反転し、送出されたことが分かる。

## カスタマイズ

- **送出キーやラベル**: [`src/main.cpp`](src/main.cpp) の `ACTIONS[]` を編集。
  - 例: 矢印+Enter方式にしたい場合は `bleKeyboard.write()` を `KEY_DOWN_ARROW` / `KEY_RETURN` の組み合わせに変更。
- **BLEデバイス名**: `BleKeyboard bleKeyboard("Cardputer Approver", ...)` の第1引数。

## 構成

```
platformio.ini   ビルド設定（espressif32 / StampS3 / 依存ライブラリ）
src/main.cpp     本体（BLEキーボード + UI + キー処理）
```
