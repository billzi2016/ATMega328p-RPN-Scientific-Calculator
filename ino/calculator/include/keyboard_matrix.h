/*
 * 文件名称：keyboard_matrix.h
 * 文件作用：声明 8x8 键盘矩阵扫描模块接口。
 * 设计意图：
 * 1. 隔离 74HC595/74HC165 的底层时序细节。
 * 2. 对上层统一输出 KeyEvent，避免业务层直接处理位图与引脚。
 */

#ifndef KEYBOARD_MATRIX_H
#define KEYBOARD_MATRIX_H

#include <Arduino.h>

#include "config.h"
#include "types.h"

namespace calculator {

class KeyboardMatrix {
 public:
  void begin();
  void update();
  bool pollEvent(KeyEvent* event);

 private:
  uint8_t current_row_state_[kKeyboardRowCount] = {};
  uint8_t stable_row_state_[kKeyboardRowCount] = {};
  uint16_t last_change_ms_[kKeyboardRowCount][kKeyboardColumnCount] = {};

  KeyEvent pending_event_ {KeyCode::kNone, false, 0};
  bool has_pending_event_ = false;
  uint16_t last_scan_ms_ = 0;

  void scanMatrix();
  void driveSingleRow(uint8_t row_index);
  uint8_t readColumns() const;
  KeyCode mapKey(uint8_t row_index, uint8_t column_index) const;
  bool isPressed(uint8_t row_mask, uint8_t column_index) const;
  void writePressedState(uint8_t* row_mask, uint8_t column_index, bool pressed);
};

}  // namespace calculator

#endif  // KEYBOARD_MATRIX_H
