#include <Arduino.h>
#include <M5Unified.h>
#include <LittleFS.h>
#include <BleKeyboard.h>
// #include <BleGamepad.h>
// #include <NimBLEDevice.h>
#include <keymap.h>

// BleGamepad bleGamepad("AtomShiftLever");

BleKeyboard bleKeyboard("AtomShiftLever");

// M5StickC
int GPIO_LEVER_UP = 26;
int GPIO_LEVER_DOWN = 0;

// M5Atom
// #define GPIO_LEVER_UP 5
// #define GPIO_LEVER_DOWN 8
int GPIO_LEVER_RIGHT = 6;
int GPIO_LEVER_LEFT = 7;

#define FORMAT_LITTLEFS_IF_FAILED true

#define BIT_LEVER_UP 0b00000001
#define BIT_LEVER_DOWN 0b00000010
#define BIT_LEVER_RIGHT 0b00000100
#define BIT_LEVER_LEFT 0b00001000

LGFX_Sprite baseSprite(&M5.Display);

int width;
int height;

uint8_t resultPast = 1;
// uint8 configMode = false;
enum ConfigMode { none = 0,
                  forward,
                  backward };

ConfigMode configMode = none;

uint8_t keyForward = 0xF0;
uint8_t keyBackward = 0xF1;
// uint8_t keyForward = KEY_LEFT_CTRL;
// uint8_t keyBackward = KEY_LEFT_SHIFT;

const char *getKeyName(uint8_t &key, bool isFoward = true) {
  uint8_t startKey = key; // Save the starting key to detect looping.
  do {
    auto it = keymapMap.find(key);
    if (it != keymapMap.end()) {
      return it->second;
    }
    if (isFoward) {
      key++;
      if (key > 0xFB) {
        key = 0x81;
      }
    }
    else {
      key--;
      if (key < 0x81) {
        key = 0xFB;
      }
    }
  } while (key != startKey);
  return nullptr;
}
uint8_t readLever() {
  uint8_t result = 0;

  if (!digitalRead(GPIO_LEVER_UP)) {
    // レバー上
    result |= BIT_LEVER_UP;
  }
  if (!digitalRead(GPIO_LEVER_DOWN)) {
    // レバー下
    result |= BIT_LEVER_DOWN;
  }
  if (!digitalRead(GPIO_LEVER_RIGHT)) {
    // 右
    result |= BIT_LEVER_RIGHT;
  }
  if (!digitalRead(GPIO_LEVER_LEFT)) {
    // 左
    result |= BIT_LEVER_LEFT;
  }
  baseSprite.setCursor(width * 0.5, height * 0.9);
  baseSprite.printf("%0d\n", result);
  return result;
}

void draw6Direction(uint8_t mode) {
  int color1 = TFT_WHITE;
  int color2 = TFT_WHITE;
  int color3 = TFT_WHITE;
  int color4 = TFT_WHITE;
  int color5 = TFT_WHITE;
  int color6 = TFT_WHITE;
  switch (mode) {
  case BIT_LEVER_UP | BIT_LEVER_LEFT:
    // 1速
    color1 = TFT_RED;
    break;
  case BIT_LEVER_DOWN | BIT_LEVER_LEFT:
    // 2速
    color2 = TFT_RED;
    break;
  case BIT_LEVER_UP | BIT_LEVER_RIGHT:
    // 5速
    color5 = TFT_RED;
    break;
  case BIT_LEVER_DOWN | BIT_LEVER_RIGHT:
    // 6速
    color6 = TFT_RED;
    break;
  case BIT_LEVER_UP:
    // 3速
    color3 = TFT_RED;
    break;
  case BIT_LEVER_DOWN:
    // 4速
    color4 = TFT_RED;
    break;
  default:
    break;
  }

  baseSprite.fillRect(width * 0.4, height * 0.4, width * 0.2, height * 0.1, TFT_WHITE);

  // 1
  baseSprite.fillTriangle(width * 0.0, height * 0.2, width * 0.3, height * 0.2, width * 0.15, height * 0.1, color1);
  baseSprite.fillRect(width * 0.05, height * 0.2, width * 0.2, height * 0.2 + 1, color1);
  baseSprite.fillRect(width * 0.05, height * 0.4, width * 0.35 + 1, height * 0.1, color1 & color2);
  // 2
  baseSprite.fillRect(width * 0.05, height * 0.5 - 1, width * 0.2, height * 0.2 + 2, color2);
  baseSprite.fillTriangle(width * 0.0, height * 0.7, width * 0.3, height * 0.7, width * 0.15, height * 0.8, color2);

  // 3
  baseSprite.fillTriangle(width * 0.35, height * 0.2, width * 0.65, height * 0.2, width * 0.5, height * 0.1, color3);
  baseSprite.fillRect(width * 0.4, height * 0.2 - 1, width * 0.2, height * 0.2 + 2, color3);

  // 4
  baseSprite.fillTriangle(width * 0.35, height * 0.7, width * 0.65, height * 0.7, width * 0.5, height * 0.8, color4);

  baseSprite.fillRect(width * 0.4, height * 0.5 - 1, width * 0.2, height * 0.2 + 2, color4);

  // 5
  baseSprite.fillTriangle(width * 0.7, height * 0.2, width * 1.0, height * 0.2, width * 0.85, height * 0.1, color5);
  baseSprite.fillRect(width * 0.75, height * 0.2, width * 0.2, height * 0.2 + 1, color5);

  baseSprite.fillRect(width * 0.6, height * 0.4, width * 0.35, height * 0.1, color5 & color6);
  // 6
  baseSprite.fillTriangle(width * 0.7, height * 0.7, width * 1.0, height * 0.7, width * 0.85, height * 0.8, color6);
  baseSprite.fillRect(width * 0.75, height * 0.5 - 1, width * 0.2, height * 0.2 + 2, color6);

  // baseSprite.fillRect(width * 0.1, height * 0.3, width * 0.2, height * 0.4, color2);
  // baseSprite.fillRect(width * 0.3, height * 0.7, width * 0.4, height * 0.2, color3);
  // baseSprite.fillRect(width * 0.7, height * 0.3, width * 0.2, height * 0.4, color4);
  // baseSprite.fillRect(width * 0.1, height * 0.1, width * 0.2, height * 0.2, color5);
  // baseSprite.fillRect(width * 0.7, height * 0.1, width * 0.2, height * 0.2, color6);
}

void drawDirection(uint8_t mode) {
  int colorU = 0;
  int colorM = 0;
  int colorB = 0;
  switch (mode) {
  case 0:
    colorU = TFT_WHITE;
    colorM = TFT_WHITE;
    colorB = TFT_WHITE;
    break;
  case 1:
    colorU = TFT_RED;
    colorM = TFT_RED;
    colorB = TFT_WHITE;
    break;
  case 2:
    colorU = TFT_WHITE;
    colorM = TFT_RED;
    colorB = TFT_RED;
    break;

  default:
    break;
  }

  baseSprite.fillTriangle(width / 2, height * 0.1, width * 0.1, height * 0.3, width * 0.9, height * 0.3, colorU);
  baseSprite.fillRect(width * 0.3, height * 0.3, width * 0.4, height * 0.3, colorM);
  baseSprite.fillTriangle(width / 2, height * 0.8, width * 0.1, height * 0.6, width * 0.9, height * 0.6, colorB);
}

void drawMode() {
  baseSprite.setTextSize(1);
  baseSprite.setCursor(0, height * 0.9);
  baseSprite.print("Seq. Mode");
}

void doConfigMode(uint8_t result) {

  baseSprite.clear();
  baseSprite.setCursor(0, height * 0.6);
  const char *leverDirection;

  if (M5.BtnA.wasPressed()) {
    // keyCodeを保存
    switch (configMode) {
    case forward:
      configMode = backward;
      break;
    case backward:
      configMode = none;
      break;
    case none:
      configMode = forward;
      break;
    default:
      break;
    }
    File file = LittleFS.open("/key.properties", "w");
    // forward, backwardの順にキーストロークを書き込む
    file.printf("%d,%d", keyForward, keyBackward);
    file.close();
  }

  switch (configMode) {
  case forward:
    leverDirection = "forward";
    break;
  case backward:
    leverDirection = "backward";
    break;
  default:
    leverDirection = "none";
    break;
  }

  baseSprite.printf("Choose key for lever %s", leverDirection);
  baseSprite.setCursor(0, height * 0.2);

  const char *keyName;
  keyName = getKeyName(configMode == forward ? keyForward : keyBackward);

  baseSprite.printf("%s", keyName);
  baseSprite.pushSprite(0, 0);

  if (result == resultPast) {
    // 入力変化なし
    return;
  }
  resultPast = result;

  if (result & BIT_LEVER_UP) {
    // keyForward++;

    configMode == forward ? keyForward++ : keyBackward++;
    if (configMode == forward ? keyForward : keyBackward > 0xFB) {
      configMode == forward ? keyForward : keyBackward = 0x81;
    }
    getKeyName(configMode == forward ? keyForward : keyBackward);
  }
  else if (result & BIT_LEVER_DOWN) {
    configMode == forward ? keyForward-- : keyBackward--;
    if (configMode == forward ? keyForward : keyBackward < 0x81) {
      configMode == forward ? keyForward : keyBackward = 0xFB;
    }
    getKeyName(configMode == forward ? keyForward : keyBackward, false);
  }
}

void drawKey() {
  baseSprite.setTextSize(1);
  baseSprite.setCursor(width * 0, height * 0);
  baseSprite.printf("%s", getKeyName(keyForward));
  baseSprite.setCursor(width * 0, height * 0.85);
  baseSprite.printf("%s", getKeyName(keyBackward));
}

void sendKey(uint8_t key) {
  // if (bleGamepad.isConnected()) {
  //   bleGamepad.press(key);
  // }
  // bleKeyboard.write(key);
}

void doHShift(uint8_t result) {

  if (result == 0 || result == BIT_LEVER_LEFT || result == BIT_LEVER_RIGHT) {
    bleKeyboard.releaseAll();
    // bleGamepad.release(BUTTON_11);
    // bleGamepad.release(BUTTON_12);
    // bleGamepad.release(BUTTON_13);
    // bleGamepad.release(BUTTON_14);
    // bleGamepad.release(BUTTON_15);
    // bleGamepad.release(BUTTON_16);
    return;
  }

  if (result & BIT_LEVER_UP && result & BIT_LEVER_RIGHT) {
    // bleKeyboard.write(keyForward);
    // draw6Direction(5);
    Serial.print("UP RIGHT \n");
    bleKeyboard.press(KEY_F11);
    // bleKeyboard.write(KEY_NUM_7);
    // sendKey(BUTTON_15);
  }
  else if (result & BIT_LEVER_UP && result & BIT_LEVER_LEFT) {
    // draw6Direction(1);
    // bleKeyboard.write(keyBackward);
    Serial.print("UP LEFT \n");
    // bleKeyboard.write(KEY_NUM_1);
    bleKeyboard.press('-');
    // sendKey(BUTTON_10);
  }
  else if (result & BIT_LEVER_DOWN && result & BIT_LEVER_RIGHT) {

    Serial.print("DOWN RIGHT \n");
    // bleKeyboard.write(KEY_NUM_9);
    bleKeyboard.press('\\');
    // draw6Direction(6);
    // sendKey(BUTTON_16);
  }
  else if (result & BIT_LEVER_DOWN && result & BIT_LEVER_LEFT) {
    Serial.print("DOWN LEFT \n");
    // bleKeyboard.write(KEY_NUM_3);
    // draw6Direction(2);
    bleKeyboard.press(',');
    // bleKeyboard.write(keyBackward);
    // sendKey(BUTTON_9);
  }
  // else if (result & BIT_LEVER_RIGHT) {
  //   bleKeyboard.write(keyForward);
  // }
  // else if (result & BIT_LEVER_LEFT) {
  //   bleKeyboard.write(keyBackward);
  // }
  else if (result & BIT_LEVER_UP) {
    Serial.print("UP \n");
    // bleKeyboard.write(KEY_NUM_8);
    bleKeyboard.press('.');
    // sendKey(BUTTON_13);
  }
  else if (result & BIT_LEVER_DOWN) {
    Serial.print("DOWN \n");
    bleKeyboard.press('/');
    // bleKeyboard.write(KEY_NUM_2);
    // sendKey(BUTTON_14);
  }

  // if (result == 0) {
  //   isRightInputed = false;
  //   isLeftInputed = false;
  // }
  // if (result & BIT_LEVER_RIGHT) {
  //   baseSprite.setCursor(width * 0.5, 0);
  //   Serial.print("RIGHT \n");
  //   isRightInputed = true;
  // }
  // else if (result & BIT_LEVER_LEFT) {
  //   baseSprite.setCursor(width * 0.5, 0);
  //   Serial.print("LEFT \n");
  //   isLeftInputed = true;
  // }
  // if (result & BIT_LEVER_UP) {
  //   if (isRightInputed) {
  //     baseSprite.setCursor(width * 0.5, 0);
  //     baseSprite.print("UP RIGHT");
  //     // bleKeyboard.write();
  //   }
  //   else if (isLeftInputed) {
  //     baseSprite.setCursor(width * 0.5, 0);
  //     baseSprite.print("UP LEFT");
  //   }
  //   else {
  //     baseSprite.setCursor(width * 0.5, 0);
  //     baseSprite.print("UP");
  //     // bleKeyboard.write(keyForward);
  //   }
  // }
  // else if (result & BIT_LEVER_DOWN) {
  //   if (isRightInputed) {
  //     baseSprite.setCursor(width * 0.5, 0);
  //     baseSprite.print("DOWN RIGHT");
  //   }
  //   else if (isLeftInputed) {
  //     baseSprite.setCursor(width * 0.5, 0);
  //     baseSprite.print("DOWN LEFT");
  //   }
  //   else {
  //     baseSprite.setCursor(width * 0.5, 0);
  //     baseSprite.print("DOWN");
  //   }
  // }
}

void doSeqShit(uint8_t result) {
  // 入力あり
  if (result & BIT_LEVER_UP) {
    Serial.print("UP \n");
    // baseSprite.print("UP ");
    // bleKeyboard.write(KEY_LEFT_CTRL);
    // bleKeyboard.write(KEY_CAPS_LOCK);
    // bleKeyboard.write(keyForward);
  }
  else if (result & BIT_LEVER_DOWN) {
    Serial.print("DOWN \n");
    // baseSprite.print("DOWN ");
    // bleKeyboard.write(KEY_LEFT_SHIFT);
    // drawDirection(2);
    // bleKeyboard.write(keyBackward);
  }
}

void setup(void) {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  // begin M5Unified.
  M5.begin(cfg);

  switch (M5.getBoard()) {
  case m5gfx::board_t::board_M5StickC:
    GPIO_LEVER_UP = 26;
    GPIO_LEVER_DOWN = 0;
    pinMode(GPIO_LEVER_UP, INPUT_PULLUP);
    pinMode(GPIO_LEVER_DOWN, INPUT);
    Serial.println("M5StickC");
    break;
  case m5gfx::board_t::board_M5AtomS3:
    GPIO_LEVER_UP = 5;
    GPIO_LEVER_DOWN = 8;
    pinMode(GPIO_LEVER_UP, INPUT_PULLUP);
    pinMode(GPIO_LEVER_DOWN, INPUT_PULLUP);
    pinMode(GPIO_LEVER_RIGHT, INPUT_PULLUP);
    pinMode(GPIO_LEVER_LEFT, INPUT_PULLUP);
    Serial.println("M5AtomS3");
    break;
  }

  bleKeyboard.begin();
  // bleGamepad.begin();

  width = M5.Display.width();
  height = M5.Display.height();

  M5.Display.setRotation(2);
  M5.Display.setTextSize(2);

  baseSprite.createSprite(width, height);
  baseSprite.fillScreen(TFT_BLACK);
  baseSprite.setTextColor(TFT_WHITE);
  baseSprite.setTextSize(2);

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  if (!LittleFS.exists("/key.properties")) {
    // ファイルが存在しない場合は作成する
    File file = LittleFS.open("/key.properties", "w");
    // forward, backwardの順にキーストロークを書き込む
    // CTRL = 0x80, SHIFT = 0x81
    file.printf("%d,%d", 0x80, 0x81);
    file.close();
  }

  File file = LittleFS.open("/key.properties", "r");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  std::string line;

  while (file.available()) { // ファイルのデータ分繰り返す
    char c = file.read();    // ファイルから1バイトずつデータを読み取り
    if (c == ',') {
      keyForward = std::stoi(line);
      Serial.printf("keyForward= 0x%0X\n", std::stoi(line));
      line = "";
    }
    else {
      line += c;
    }
    // Serial.printf("line= %s\n", line.c_str());
  }
  keyBackward = std::stoi(line);
  Serial.printf("keyBackward= 0x%0X\n", std::stoi(line));
  Serial.println("property read done");
  file.close();

  // baseSprite.pushSprite(0, 0);
}

void loop(void) {
  M5.update();
  delay(1);

  uint8_t result = readLever();

  if (M5.BtnA.wasHold()) {
    configMode = forward;
    Serial.println("configMode = forward");
  }

  if (configMode != none) {
    doConfigMode(result);
    return;
  }

  baseSprite.clear();

  drawKey();

  drawMode();

  // drawDirection(result);
  draw6Direction(result);

  baseSprite.pushSprite(0, 0);

  if (result == resultPast) {
    // 入力変化なし
    return;
  }
  resultPast = result;

  // doSeqShit(result);
  doHShift(result);

  baseSprite.pushSprite(0, 0);
}
