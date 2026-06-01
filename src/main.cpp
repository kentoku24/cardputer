// =============================================================================
//  M5Stack Cardputer — Claude Code 承認用 BLEキーボード
// -----------------------------------------------------------------------------
//  Cardputer を BLE キーボードとして PC/Mac にペアリングし、
//  Claude Code の権限プロンプトに対して以下の3操作だけを送出する:
//
//    [1] 承認        → "1" を送出 (Yes)
//    [2] 常に承認    → "2" を送出 (Yes, and don't ask again)
//    [3] 不承認      → "3" を送出 (No)
//
//  Claude Code の TUI は数字キーで選択肢を即決定できるため、
//  数字 "1"/"2"/"3" を1打鍵送るだけで承認/常に承認/不承認が確定する。
//
//  操作: Cardputer キーボードの 1 / 2 / 3 キーを押す。
// =============================================================================

// 注意: include 順が重要。
//   BleKeyboard.h は KEY_LEFT_CTRL 等を enum で定義する。
//   M5Cardputer の Keyboard_def.h は同名を #define で定義する。
//   M5Cardputer を先に include すると、マクロが BleKeyboard の enum を
//   壊して "expected unqualified-id before numeric constant" になる。
//   そのため BleKeyboard.h を先に include して enum を確定させる。
#include <BleKeyboard.h>
#include <M5Cardputer.h>

// BLE デバイス名 / メーカー名 / 初期バッテリ残量
BleKeyboard bleKeyboard("Cardputer Approver", "M5Stack", 100);

// -----------------------------------------------------------------------------
//  アクション定義
// -----------------------------------------------------------------------------
struct Action {
  char     key;       // Cardputer 側の押下キー & 送出する文字 (1:1)
  const char* label;  // 画面表示ラベル
  uint16_t color;     // 行の色
};

static const Action ACTIONS[] = {
  {'1', "1: 承認",     TFT_GREEN},
  {'2', "2: 常に承認", TFT_CYAN},
  {'3', "3: 不承認",   TFT_RED},
};
static const int NUM_ACTIONS = sizeof(ACTIONS) / sizeof(ACTIONS[0]);

// 直近に送出したアクションのインデックス (-1 = なし)。フィードバック表示に使用。
static int  g_lastSent     = -1;
static bool g_lastConn     = false;  // 直近の接続状態 (再描画判定用)
static bool g_needRedraw   = true;

// -----------------------------------------------------------------------------
//  画面描画
// -----------------------------------------------------------------------------
void drawUI() {
  auto& d = M5Cardputer.Display;
  const bool connected = bleKeyboard.isConnected();

  d.fillScreen(TFT_BLACK);

  // --- タイトルバー -------------------------------------------------------
  d.fillRect(0, 0, d.width(), 22, TFT_DARKGREY);
  d.setFont(&fonts::efontJA_16);
  d.setTextColor(TFT_WHITE, TFT_DARKGREY);
  d.setCursor(4, 3);
  d.print("Claude Code 承認");

  // 接続状態インジケータ (右上)
  d.setTextColor(connected ? TFT_GREENYELLOW : TFT_ORANGE, TFT_DARKGREY);
  d.setCursor(d.width() - 70, 3);
  d.print(connected ? "● 接続中" : "○ 待機中");

  // --- 3つのアクション行 --------------------------------------------------
  const int top   = 26;
  const int rowH  = 30;
  const int gap   = 4;
  d.setFont(&fonts::efontJA_24);

  for (int i = 0; i < NUM_ACTIONS; i++) {
    const int y = top + i * (rowH + gap);
    const bool flash = (i == g_lastSent);

    d.fillRoundRect(4, y, d.width() - 8, rowH, 5,
                    flash ? TFT_WHITE : ACTIONS[i].color);
    d.setTextColor(flash ? ACTIONS[i].color : TFT_BLACK,
                   flash ? TFT_WHITE : ACTIONS[i].color);
    d.setCursor(14, y + 4);
    d.print(ACTIONS[i].label);
  }

  d.setFont(&fonts::efontJA_16);
}

// -----------------------------------------------------------------------------
//  setup
// -----------------------------------------------------------------------------
void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg);

  M5Cardputer.Display.setRotation(1);          // 横向き (240x135)
  M5Cardputer.Display.setTextSize(1);

  Serial.begin(115200);
  Serial.println("[Cardputer Approver] starting BLE keyboard...");

  bleKeyboard.begin();
  drawUI();
}

// -----------------------------------------------------------------------------
//  指定アクションを BLE 経由で送出
// -----------------------------------------------------------------------------
void sendAction(int idx) {
  if (idx < 0 || idx >= NUM_ACTIONS) return;

  if (!bleKeyboard.isConnected()) {
    Serial.println("[skip] BLE not connected");
    // 未接続でも UI で一瞬フィードバックは出す
    g_lastSent = idx;
    drawUI();
    delay(150);
    g_lastSent = -1;
    drawUI();
    return;
  }

  bleKeyboard.write(ACTIONS[idx].key);
  Serial.printf("[sent] '%c' (%s)\n", ACTIONS[idx].key, ACTIONS[idx].label);

  // 送出フィードバック (該当行を白く反転)
  g_lastSent = idx;
  drawUI();
  delay(180);
  g_lastSent = -1;
  drawUI();
}

// -----------------------------------------------------------------------------
//  loop
// -----------------------------------------------------------------------------
void loop() {
  M5Cardputer.update();

  // 接続状態が変化したら再描画
  const bool connected = bleKeyboard.isConnected();
  if (connected != g_lastConn) {
    g_lastConn   = connected;
    g_needRedraw = true;
  }
  if (g_needRedraw) {
    g_needRedraw = false;
    drawUI();
  }

  // キー入力処理: 押された瞬間のみ反応
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    for (char c : status.word) {
      for (int i = 0; i < NUM_ACTIONS; i++) {
        if (c == ACTIONS[i].key) {
          sendAction(i);
        }
      }
    }
  }

  delay(10);
}
