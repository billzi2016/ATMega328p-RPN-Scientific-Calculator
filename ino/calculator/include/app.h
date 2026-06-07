/*
 * 文件名称：app.h
 * 文件作用：声明应用层主状态机接口。
 * 设计意图：
 * 1. 作为整个系统的调度中心，负责串联键盘、显示、计算核心和存储。
 * 2. 让 .ino 入口保持极简，后续所有复杂逻辑都收敛到 App 层。
 */

#ifndef CALCULATOR_APP_H
#define CALCULATOR_APP_H

#include <Arduino.h>

#include "calculator_core.h"
#include "config.h"
#include "diagnostics.h"
#include "display_lcd2004.h"
#include "keyboard_matrix.h"
#include "storage_sd.h"
#include "types.h"

namespace calculator {

class App {
 public:
  void begin();
  void update();

 private:
  AppScreen screen_ = AppScreen::kHome;
  AngleMode angle_mode_ = AngleMode::kDeg;
  NumberBase number_base_ = NumberBase::kDec;

  CalculatorCore core_;
  KeyboardMatrix keyboard_;
  DisplayLcd2004 display_;
  StorageSd storage_;
  Diagnostics diagnostics_;

  char input_buffer_[kInputBufferSize] = {};
  uint8_t input_length_ = 0;
  uint16_t step_counter_ = 0;
  char last_expression_[kInputBufferSize] = {};
  char last_result_[kResultBufferSize] = {};

  void handleKeyEvent(const KeyEvent& event);
  void handleHomeKey(KeyCode code);
  void renderCurrentScreen();
  void renderMenuScreen();
  void renderHistoryScreen();
  void renderFileScreen();
  void appendInputChar(char ch);
  void clearInput();
  bool commitInputIfNeeded();
  void performBinaryOperation(BinaryOp op, const char* expression_text);
  void logResult(const char* expression_text);
  void buildStatusText(char* out_text, size_t length) const;
  void setLastExpression(const char* expression_text);
};

}  // namespace calculator

#endif  // CALCULATOR_APP_H
