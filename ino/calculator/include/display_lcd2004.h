/*
 * 文件名称：display_lcd2004.h
 * 文件作用：声明 LCD2004 显示模块接口。
 * 设计意图：
 * 1. 统一封装页面渲染逻辑，避免业务模块直接调用底层 LCD API。
 * 2. 让主界面、历史界面、错误提示后续都能按页面方式扩展。
 */

#ifndef DISPLAY_LCD2004_H
#define DISPLAY_LCD2004_H

#include <Arduino.h>

#include "types.h"

namespace calculator {

class DisplayLcd2004 {
 public:
  void begin();
  void showBootScreen();
  void renderHome(const AppViewModel& model);
  void showMessage(const char* line1, const char* line2);
  void showMessage(const __FlashStringHelper* line1, const __FlashStringHelper* line2);
};

}  // namespace calculator

#endif  // DISPLAY_LCD2004_H
