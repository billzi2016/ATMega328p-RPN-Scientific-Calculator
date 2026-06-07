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
  /*
   * 函数作用：初始化应用层以及全部子模块。
   */
  void begin();

  /*
   * 函数作用：执行一次主循环更新。
   */
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
  char last_result_[kResultBufferSize] = {};
  uint16_t history_session_index_ = 0;
  uint8_t history_view_offset_ = 0;
  uint16_t file_browser_session_index_ = 0;

  /*
   * 函数作用：按当前页面状态分发按键事件。
   */
  void handleKeyEvent(const KeyEvent& event);

  /*
   * 函数作用：处理主计算界面的按键逻辑。
   */
  void handleHomeKey(KeyCode code);

  /*
   * 函数作用：根据当前页面状态刷新 LCD。
   */
  void renderCurrentScreen();

  /*
   * 函数作用：渲染菜单界面。
   */
  void renderMenuScreen();

  /*
   * 函数作用：从 SD 读取历史并渲染历史界面。
   */
  void renderHistoryScreen();

  /*
   * 函数作用：渲染日志文件状态界面。
   */
  void renderFileScreen();

  /*
   * 函数作用：向当前输入缓冲区追加一个字符。
   */
  void appendInputChar(char ch);

  /*
   * 函数作用：清空当前输入缓冲区。
   */
  void clearInput();

  /*
   * 函数作用：若输入缓冲区非空，则把文本转换为数值并压栈。
   */
  bool commitInputIfNeeded();

  /*
   * 函数作用：执行二元运算并在成功后记录历史。
   */
  void performBinaryOperation(BinaryOp op, const char* expression_text);

  /*
   * 函数作用：执行一元运算并在成功后记录历史。
   */
  void performUnaryOperation(bool succeeded, const char* expression_text);

  /*
   * 函数作用：把当前计算结果写入运行态缓存，并追加到 SD 历史文件。
   */
  void logResult(const char* expression_text);

  /*
   * 函数作用：构造主界面底部状态行。
   */
  void buildStatusText(char* out_text, size_t length) const;
};

}  // namespace calculator

#endif  // CALCULATOR_APP_H
