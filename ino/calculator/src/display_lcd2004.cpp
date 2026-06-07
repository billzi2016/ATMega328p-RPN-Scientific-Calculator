/*
 * 文件名称：display_lcd2004.cpp
 * 文件作用：实现 LCD2004 页面显示。
 * 设计意图：
 * 1. 当前先完成主界面与通用消息界面，确保系统可见性。
 * 2. 后续可在这里继续扩展历史浏览、文件浏览与菜单页面。
 */

#include "../include/display_lcd2004.h"

#include <LiquidCrystal.h>

#include "../include/config.h"
#include "../include/pins.h"

namespace calculator {

namespace {

LiquidCrystal g_lcd(
    kLcdPinRs,
    kLcdPinE,
    kLcdPinD4,
    kLcdPinD5,
    kLcdPinD6,
    kLcdPinD7);

}  // namespace

/*
 * 函数作用：初始化 LCD2004。
 */
void DisplayLcd2004::begin() {
  g_lcd.begin(kDisplayColumns, kDisplayRows);
  g_lcd.clear();
}

/*
 * 函数作用：显示启动页。
 */
void DisplayLcd2004::showBootScreen() {
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  g_lcd.print(F("Mega2560 RPN"));
  g_lcd.setCursor(0, 1);
  g_lcd.print(F("Calculator Init"));
  g_lcd.setCursor(0, 2);
  g_lcd.print(F("LCD/KEY/SD Check"));
  g_lcd.setCursor(0, 3);
  g_lcd.print(F("Please Wait..."));
}

/*
 * 函数作用：渲染主界面。
 */
void DisplayLcd2004::renderHome(const AppViewModel& model) {
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  g_lcd.print(F("Mega2560 RPN Calc"));
  g_lcd.setCursor(0, 1);
  g_lcd.print(model.input_line);
  g_lcd.setCursor(0, 2);
  g_lcd.print(model.result_line);
  g_lcd.setCursor(0, 3);
  g_lcd.print(model.status_line);
}

/*
 * 函数作用：显示普通字符串消息页。
 */
void DisplayLcd2004::showMessage(const char* line1, const char* line2) {
  g_lcd.clear();
  g_lcd.setCursor(0, 1);
  g_lcd.print(line1 == nullptr ? "" : line1);
  g_lcd.setCursor(0, 2);
  g_lcd.print(line2 == nullptr ? "" : line2);
}

/*
 * 函数作用：显示 Flash 字符串消息页。
 */
void DisplayLcd2004::showMessage(const __FlashStringHelper* line1, const __FlashStringHelper* line2) {
  g_lcd.clear();
  g_lcd.setCursor(0, 1);
  g_lcd.print(line1 == nullptr ? F("") : line1);
  g_lcd.setCursor(0, 2);
  g_lcd.print(line2 == nullptr ? F("") : line2);
}

}  // namespace calculator
