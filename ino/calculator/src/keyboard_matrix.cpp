/*
 * 文件名称：keyboard_matrix.cpp
 * 文件作用：实现 8x8 键盘矩阵扫描。
 * 设计意图：
 * 1. 使用 74HC595 输出行选择，使用 74HC165 读取列状态。
 * 2. 采用“行低有效、列默认高、按下读低”的扫描逻辑，保持与文档一致。
 */

#include "../include/keyboard_matrix.h"

#include <avr/pgmspace.h>

#include "../include/pins.h"

namespace calculator {

namespace {

const uint8_t kKeyMap[kKeyboardRowCount][kKeyboardColumnCount] PROGMEM = {
    {static_cast<uint8_t>(KeyCode::kOn), static_cast<uint8_t>(KeyCode::kOff), static_cast<uint8_t>(KeyCode::kMenu), static_cast<uint8_t>(KeyCode::kMode), static_cast<uint8_t>(KeyCode::kHist), static_cast<uint8_t>(KeyCode::kFile), static_cast<uint8_t>(KeyCode::kUp), static_cast<uint8_t>(KeyCode::kDel)},
    {static_cast<uint8_t>(KeyCode::kShift), static_cast<uint8_t>(KeyCode::kAlpha), static_cast<uint8_t>(KeyCode::kLeft), static_cast<uint8_t>(KeyCode::kOk), static_cast<uint8_t>(KeyCode::kRight), static_cast<uint8_t>(KeyCode::kLParen), static_cast<uint8_t>(KeyCode::kRParen), static_cast<uint8_t>(KeyCode::kBack)},
    {static_cast<uint8_t>(KeyCode::k7), static_cast<uint8_t>(KeyCode::k8), static_cast<uint8_t>(KeyCode::k9), static_cast<uint8_t>(KeyCode::kDivide), static_cast<uint8_t>(KeyCode::kSin), static_cast<uint8_t>(KeyCode::kCos), static_cast<uint8_t>(KeyCode::kTan), static_cast<uint8_t>(KeyCode::kPi)},
    {static_cast<uint8_t>(KeyCode::k4), static_cast<uint8_t>(KeyCode::k5), static_cast<uint8_t>(KeyCode::k6), static_cast<uint8_t>(KeyCode::kMultiply), static_cast<uint8_t>(KeyCode::kLn), static_cast<uint8_t>(KeyCode::kLog), static_cast<uint8_t>(KeyCode::kPowXY), static_cast<uint8_t>(KeyCode::kE)},
    {static_cast<uint8_t>(KeyCode::k1), static_cast<uint8_t>(KeyCode::k2), static_cast<uint8_t>(KeyCode::k3), static_cast<uint8_t>(KeyCode::kMinus), static_cast<uint8_t>(KeyCode::kSqrt), static_cast<uint8_t>(KeyCode::kSquare), static_cast<uint8_t>(KeyCode::kReciprocal), static_cast<uint8_t>(KeyCode::kPercent)},
    {static_cast<uint8_t>(KeyCode::k0), static_cast<uint8_t>(KeyCode::kDot), static_cast<uint8_t>(KeyCode::kSign), static_cast<uint8_t>(KeyCode::kPlus), static_cast<uint8_t>(KeyCode::kExp), static_cast<uint8_t>(KeyCode::kEng), static_cast<uint8_t>(KeyCode::kAns), static_cast<uint8_t>(KeyCode::kEqual)},
    {static_cast<uint8_t>(KeyCode::kEnter), static_cast<uint8_t>(KeyCode::kSwap), static_cast<uint8_t>(KeyCode::kDrop), static_cast<uint8_t>(KeyCode::kDup), static_cast<uint8_t>(KeyCode::kRollUp), static_cast<uint8_t>(KeyCode::kRollDown), static_cast<uint8_t>(KeyCode::kSto), static_cast<uint8_t>(KeyCode::kRcl)},
    {static_cast<uint8_t>(KeyCode::kMc), static_cast<uint8_t>(KeyCode::kMr), static_cast<uint8_t>(KeyCode::kMPlus), static_cast<uint8_t>(KeyCode::kMMinus), static_cast<uint8_t>(KeyCode::kDeg), static_cast<uint8_t>(KeyCode::kRad), static_cast<uint8_t>(KeyCode::kBin), static_cast<uint8_t>(KeyCode::kHex)},
};

}  // namespace

/*
 * 函数作用：初始化 595/165 相关引脚方向与默认电平。
 */
void KeyboardMatrix::begin() {
  pinMode(kShift595Ser, OUTPUT);
  pinMode(kShift595Clock, OUTPUT);
  pinMode(kShift595Latch, OUTPUT);

  pinMode(kShift165Load, OUTPUT);
  pinMode(kShift165Clock, OUTPUT);
  pinMode(kShift165Data, INPUT);

  digitalWrite(kShift595Ser, LOW);
  digitalWrite(kShift595Clock, LOW);
  digitalWrite(kShift595Latch, LOW);
  digitalWrite(kShift165Load, HIGH);
  digitalWrite(kShift165Clock, LOW);
}

/*
 * 函数作用：按固定扫描周期触发一次矩阵采样。
 */
void KeyboardMatrix::update() {
  const uint16_t now = static_cast<uint16_t>(millis());
  if (static_cast<uint16_t>(now - last_scan_ms_) < kKeyboardScanIntervalMs) {
    return;
  }

  last_scan_ms_ = now;
  scanMatrix();
}

/*
 * 函数作用：取出一条已经完成去抖的按键事件。
 */
bool KeyboardMatrix::pollEvent(KeyEvent* event) {
  if (!has_pending_event_ || event == nullptr) {
    return false;
  }

  *event = pending_event_;
  has_pending_event_ = false;
  pending_event_ = {KeyCode::kNone, false, 0};
  return true;
}

/*
 * 函数作用：扫描全部 8 行并更新稳定按键状态。
 */
void KeyboardMatrix::scanMatrix() {
  const uint16_t now = static_cast<uint16_t>(millis());

  for (uint8_t row = 0; row < kKeyboardRowCount; ++row) {
    driveSingleRow(row);
    const uint8_t columns = readColumns();

    for (uint8_t column = 0; column < kKeyboardColumnCount; ++column) {
      // 采用列上拉方案，因此读到 0 表示该列在当前行被按下。
      const bool pressed = ((columns >> column) & 0x01) == 0;
      const bool current_pressed = isPressed(current_row_state_[row], column);

      if (pressed != current_pressed) {
        writePressedState(&current_row_state_[row], column, pressed);
        last_change_ms_[row][column] = now;
      }

      if (static_cast<uint16_t>(now - last_change_ms_[row][column]) < kKeyboardDebounceMs) {
        continue;
      }

      const bool stable_pressed = isPressed(stable_row_state_[row], column);
      const bool latest_pressed = isPressed(current_row_state_[row], column);
      if (stable_pressed == latest_pressed) {
        continue;
      }

      writePressedState(&stable_row_state_[row], column, latest_pressed);

      if (!has_pending_event_) {
        pending_event_.code = mapKey(row, column);
        pending_event_.pressed = latest_pressed;
        pending_event_.timestamp_ms = now;
        has_pending_event_ = true;
      }
    }
  }
}

/*
 * 函数作用：仅选通一行，供 74HC165 读取对应列状态。
 */
void KeyboardMatrix::driveSingleRow(uint8_t row_index) {
  uint8_t row_mask = 0xFF;

  // 仅把当前行拉低，其余行保持高电平。
  row_mask &= ~(1 << row_index);

  digitalWrite(kShift595Latch, LOW);
  shiftOut(kShift595Ser, kShift595Clock, MSBFIRST, row_mask);
  digitalWrite(kShift595Latch, HIGH);
}

/*
 * 函数作用：锁存并读取当前被选中行的 8 位列输入。
 */
uint8_t KeyboardMatrix::readColumns() const {
  digitalWrite(kShift165Load, LOW);
  delayMicroseconds(5);
  digitalWrite(kShift165Load, HIGH);

  return shiftIn(kShift165Data, kShift165Clock, LSBFIRST);
}

/*
 * 函数作用：把行列坐标映射为逻辑键值。
 */
KeyCode KeyboardMatrix::mapKey(uint8_t row_index, uint8_t column_index) const {
  if (row_index >= kKeyboardRowCount || column_index >= kKeyboardColumnCount) {
    return KeyCode::kNone;
  }
  return static_cast<KeyCode>(pgm_read_byte(&kKeyMap[row_index][column_index]));
}

/*
 * 函数作用：判断指定列在位图中是否处于按下状态。
 */
bool KeyboardMatrix::isPressed(uint8_t row_mask, uint8_t column_index) const {
  return (row_mask & static_cast<uint8_t>(1U << column_index)) != 0;
}

/*
 * 函数作用：在行位图中写入某一列的按下/释放状态。
 */
void KeyboardMatrix::writePressedState(uint8_t* row_mask, uint8_t column_index, bool pressed) {
  if (row_mask == nullptr) {
    return;
  }

  const uint8_t bit_mask = static_cast<uint8_t>(1U << column_index);
  if (pressed) {
    *row_mask |= bit_mask;
  } else {
    *row_mask &= static_cast<uint8_t>(~bit_mask);
  }
}

}  // namespace calculator
