/*
 * 文件名称：app.cpp
 * 文件作用：实现应用层主状态机。
 * 设计意图：
 * 1. 把输入事件、计算逻辑、界面刷新、存储记录统一收敛到一个调度层。
 * 2. 提供可运行的主界面、菜单、历史浏览、文件浏览与基础计算链路。
 */

#include "../include/app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace calculator {

/*
 * 函数作用：初始化应用层以及全部子模块。
 */
void App::begin() {
  diagnostics_.begin();
  keyboard_.begin();
  display_.begin();
  display_.showBootScreen();
  core_.begin();
  storage_.begin();
  work_mode_ = WorkMode::kScientific;
  angle_mode_ = AngleMode::kDeg;
  number_base_ = NumberBase::kDec;
  screen_ = AppScreen::kHome;
  clearInput();
  history_session_index_ = storage_.latestSessionIndex();
  history_view_offset_ = 0;
  file_browser_session_index_ = storage_.latestSessionIndex();
  renderCurrentScreen();
}

/*
 * 函数作用：执行一次主循环更新。
 */
void App::update() {
  keyboard_.update();

  KeyEvent event;
  if (keyboard_.pollEvent(&event)) {
    handleKeyEvent(event);
    renderCurrentScreen();
  }

  delay(kMainLoopDelayMs);
}

/*
 * 函数作用：按当前页面状态分发按键事件。
 */
void App::handleKeyEvent(const KeyEvent& event) {
  if (!event.pressed) {
    return;
  }

  if (event.code == KeyCode::kShift) {
    if (screen_ == AppScreen::kStandby) {
      return;
    }
    shift_armed_ = !shift_armed_;
    alpha_armed_ = false;
    return;
  }

  if (event.code == KeyCode::kAlpha) {
    if (screen_ == AppScreen::kStandby) {
      return;
    }
    alpha_armed_ = !alpha_armed_;
    shift_armed_ = false;
    return;
  }

  switch (screen_) {
    case AppScreen::kHome:
      handleHomeKey(event.code);
      break;
    case AppScreen::kStandby:
      if (event.code == KeyCode::kOn) {
        screen_ = work_mode_ == WorkMode::kMaintenance ? AppScreen::kMaintenance : AppScreen::kHome;
      }
      break;
    case AppScreen::kMenu:
      {
        bool modifier_handled = false;
      if (shift_armed_) {
        if (event.code == KeyCode::kLeft) {
          angle_mode_ = AngleMode::kDeg;
          number_base_ = NumberBase::kDec;
          modifier_handled = true;
        } else if (event.code == KeyCode::kRight) {
          angle_mode_ = AngleMode::kRad;
          number_base_ = NumberBase::kHex;
          modifier_handled = true;
        }
        clearModifierArms();
        if (modifier_handled) {
          return;
        }
      }
      if (event.code == KeyCode::kBack || event.code == KeyCode::kMenu) {
        screen_ = AppScreen::kHome;
      } else if (event.code == KeyCode::kMode) {
        cycleWorkMode();
      } else if (event.code == KeyCode::kDeg) {
        angle_mode_ = AngleMode::kDeg;
      } else if (event.code == KeyCode::kRad) {
        angle_mode_ = AngleMode::kRad;
      } else if (event.code == KeyCode::kBin) {
        number_base_ = NumberBase::kBin;
      } else if (event.code == KeyCode::kFile) {
        screen_ = AppScreen::kMaintenance;
      } else if (event.code == KeyCode::kHex) {
        number_base_ = NumberBase::kHex;
      } else if (event.code == KeyCode::kAns) {
        number_base_ = NumberBase::kDec;
      }
      break;
      }
    case AppScreen::kHistory:
      {
        bool modifier_handled = false;
      if (shift_armed_) {
        if (event.code == KeyCode::kHist) {
          history_session_index_ = storage_.latestSessionIndex();
          history_view_offset_ = 0;
          modifier_handled = true;
        } else if (event.code == KeyCode::kLeft && storage_.availableSessionCount() > 0) {
          history_session_index_ = 1;
          history_view_offset_ = 0;
          modifier_handled = true;
        } else if (event.code == KeyCode::kRight && storage_.availableSessionCount() > 0) {
          history_session_index_ = storage_.availableSessionCount();
          history_view_offset_ = 0;
          modifier_handled = true;
        } else if (event.code == KeyCode::kUp) {
          uint8_t available_count = 0;
          uint16_t step = 0;
          char expression[kHistoryExpressionSize] = {};
          char result[kHistoryResultSize] = {};

          while (storage_.readRecentRecordFromSession(
              history_session_index_,
              history_view_offset_,
              &step,
              expression,
              sizeof(expression),
              result,
              sizeof(result),
              &available_count)) {
            if (history_view_offset_ + 1 >= available_count) {
              break;
            }
            ++history_view_offset_;
          }
          modifier_handled = true;
        }
        clearModifierArms();
        if (modifier_handled) {
          return;
        }
      }
      if (event.code == KeyCode::kBack || event.code == KeyCode::kHist) {
        screen_ = AppScreen::kHome;
      } else if (event.code == KeyCode::kUp) {
        // 历史界面向更早的记录翻页时，直接试探性读取选中文件中更早的一条记录。
        uint16_t step = 0;
        char expression[kHistoryExpressionSize] = {};
        char result[kHistoryResultSize] = {};
        uint8_t available_count = 0;
        if (storage_.readRecentRecordFromSession(
                history_session_index_,
                static_cast<uint8_t>(history_view_offset_ + 1),
                &step,
                expression,
                sizeof(expression),
                result,
                sizeof(result),
                &available_count)) {
          ++history_view_offset_;
        }
      } else if (event.code == KeyCode::kLParen && history_view_offset_ > 0) {
        --history_view_offset_;
      } else if (event.code == KeyCode::kRParen) {
        uint16_t step = 0;
        char expression[kHistoryExpressionSize] = {};
        char result[kHistoryResultSize] = {};
        uint8_t available_count = 0;
        if (storage_.readRecentRecordFromSession(
                history_session_index_,
                static_cast<uint8_t>(history_view_offset_ + 1),
                &step,
                expression,
                sizeof(expression),
                result,
                sizeof(result),
                &available_count)) {
          ++history_view_offset_;
        }
      } else if (event.code == KeyCode::kDel && history_view_offset_ > 0) {
        --history_view_offset_;
      } else if (event.code == KeyCode::kLeft && history_session_index_ > 1) {
        --history_session_index_;
        history_view_offset_ = 0;
      } else if (event.code == KeyCode::kRight && history_session_index_ < storage_.availableSessionCount()) {
        ++history_session_index_;
        history_view_offset_ = 0;
      } else if (event.code == KeyCode::kOk) {
        uint16_t step = 0;
        char expression[kHistoryExpressionSize] = {};
        char result[kHistoryResultSize] = {};
        uint8_t available_count = 0;
        if (storage_.readRecentRecordFromSession(
                history_session_index_,
                history_view_offset_,
                &step,
                expression,
                sizeof(expression),
                result,
                sizeof(result),
                &available_count)) {
          core_.push(static_cast<float>(atof(result)));
          strncpy(last_result_, result, sizeof(last_result_) - 1);
          last_result_[sizeof(last_result_) - 1] = '\0';
          screen_ = AppScreen::kHome;
        }
      }
      break;
      }
    case AppScreen::kFileBrowser:
      {
        bool modifier_handled = false;
      if (shift_armed_) {
        if (event.code == KeyCode::kLeft && storage_.availableSessionCount() > 0) {
          file_browser_session_index_ = 1;
          modifier_handled = true;
        } else if (event.code == KeyCode::kRight && storage_.availableSessionCount() > 0) {
          file_browser_session_index_ = storage_.availableSessionCount();
          modifier_handled = true;
        } else if (event.code == KeyCode::kFile) {
          file_browser_session_index_ = storage_.latestSessionIndex();
          modifier_handled = true;
        }
        clearModifierArms();
        if (modifier_handled) {
          return;
        }
      }
      if (event.code == KeyCode::kBack || event.code == KeyCode::kFile) {
        screen_ = AppScreen::kHome;
      } else if (event.code == KeyCode::kLeft && file_browser_session_index_ > 1) {
        --file_browser_session_index_;
      } else if (event.code == KeyCode::kLParen && file_browser_session_index_ > 1) {
        --file_browser_session_index_;
      } else if (event.code == KeyCode::kRight && file_browser_session_index_ < storage_.availableSessionCount()) {
        ++file_browser_session_index_;
      } else if (event.code == KeyCode::kRParen && file_browser_session_index_ < storage_.availableSessionCount()) {
        ++file_browser_session_index_;
      } else if (event.code == KeyCode::kUp) {
        file_browser_session_index_ = storage_.latestSessionIndex();
      } else if (event.code == KeyCode::kDel && storage_.availableSessionCount() > 0) {
        file_browser_session_index_ = 1;
      } else if (event.code == KeyCode::kOk && file_browser_session_index_ > 0) {
        history_session_index_ = file_browser_session_index_;
        history_view_offset_ = 0;
        screen_ = AppScreen::kHistory;
      }
      break;
      }
    case AppScreen::kMaintenance:
      if (event.code == KeyCode::kBack || event.code == KeyCode::kMenu) {
        screen_ = AppScreen::kHome;
      } else if (event.code == KeyCode::kMode) {
        cycleWorkMode();
      } else if (event.code == KeyCode::kHist) {
        history_session_index_ = storage_.latestSessionIndex();
        history_view_offset_ = 0;
        screen_ = AppScreen::kHistory;
      } else if (event.code == KeyCode::kFile) {
        file_browser_session_index_ = storage_.latestSessionIndex();
        screen_ = AppScreen::kFileBrowser;
      }
      break;
  }
}

/*
 * 函数作用：处理主计算界面的按键逻辑。
 */
void App::handleHomeKey(KeyCode code) {
  if (pending_register_action_ != PendingRegisterAction::kNone) {
    if (code == KeyCode::kBack || code == KeyCode::kDel) {
      pending_register_action_ = PendingRegisterAction::kNone;
      return;
    }
  }

  if (pending_register_action_ != PendingRegisterAction::kNone && handlePendingRegisterDigit(code)) {
    return;
  }

  if (shift_armed_) {
    const bool handled = handleShiftedHomeKey(code);
    clearModifierArms();
    if (handled) {
      return;
    }
  }

  if (alpha_armed_) {
    const bool handled = handleAlphaHomeKey(code);
    clearModifierArms();
    if (handled) {
      return;
    }
  }

  switch (code) {
    case KeyCode::k0: appendInputChar('0'); break;
    case KeyCode::k1: appendInputChar('1'); break;
    case KeyCode::k2: appendInputChar('2'); break;
    case KeyCode::k3: appendInputChar('3'); break;
    case KeyCode::k4: appendInputChar('4'); break;
    case KeyCode::k5: appendInputChar('5'); break;
    case KeyCode::k6: appendInputChar('6'); break;
    case KeyCode::k7: appendInputChar('7'); break;
    case KeyCode::k8: appendInputChar('8'); break;
    case KeyCode::k9: appendInputChar('9'); break;
    case KeyCode::kDot: appendInputChar('.'); break;
    case KeyCode::kExp: appendInputChar('e'); break;
    case KeyCode::kOn:
      screen_ = work_mode_ == WorkMode::kMaintenance ? AppScreen::kMaintenance : AppScreen::kHome;
      break;
    case KeyCode::kOff:
      clearModifierArms();
      pending_register_action_ = PendingRegisterAction::kNone;
      screen_ = AppScreen::kStandby;
      break;
    case KeyCode::kDel:
      if (input_length_ > 0) {
        --input_length_;
        input_buffer_[input_length_] = '\0';
      }
      break;
    case KeyCode::kEnter:
      if (commitInputIfNeeded()) {
        logResult("ENTER");
      } else {
        core_.enter();
        logResult("ENTER DUP");
      }
      break;
    case KeyCode::kEqual:
      if (commitInputIfNeeded()) {
        logResult("=");
      }
      break;
    case KeyCode::kPlus:
      performBinaryOperation(BinaryOp::kAdd, "+");
      break;
    case KeyCode::kMinus:
      performBinaryOperation(BinaryOp::kSubtract, "-");
      break;
    case KeyCode::kMultiply:
      performBinaryOperation(BinaryOp::kMultiply, "*");
      break;
    case KeyCode::kDivide:
      performBinaryOperation(BinaryOp::kDivide, "/");
      break;
    case KeyCode::kSign:
      if (commitInputIfNeeded()) {
        core_.toggleSign();
      } else {
        appendInputChar('-');
      }
      break;
    case KeyCode::kDrop:
      core_.drop();
      clearInput();
      break;
    case KeyCode::kSwap:
      core_.swap();
      break;
    case KeyCode::kDup:
      core_.dup();
      break;
    case KeyCode::kRollUp:
      core_.rollUp();
      break;
    case KeyCode::kRollDown:
      core_.rollDown();
      break;
    case KeyCode::kSto:
      commitInputIfNeeded();
      core_.store();
      logResult("STO");
      break;
    case KeyCode::kRcl:
      if (core_.recall()) {
        logResult("RCL");
      }
      break;
    case KeyCode::kMc:
      core_.memoryClear();
      break;
    case KeyCode::kMr:
      if (core_.push(core_.memory())) {
        logResult("MR");
      }
      break;
    case KeyCode::kMPlus:
      commitInputIfNeeded();
      core_.memoryAdd();
      break;
    case KeyCode::kMMinus:
      commitInputIfNeeded();
      core_.memorySubtract();
      break;
    case KeyCode::kSin:
      commitInputIfNeeded();
      performUnaryOperation(core_.trigSin(angle_mode_), "SIN");
      break;
    case KeyCode::kCos:
      commitInputIfNeeded();
      performUnaryOperation(core_.trigCos(angle_mode_), "COS");
      break;
    case KeyCode::kTan:
      commitInputIfNeeded();
      performUnaryOperation(core_.trigTan(angle_mode_), "TAN");
      break;
    case KeyCode::kLn:
      commitInputIfNeeded();
      performUnaryOperation(core_.naturalLog(), "LN");
      break;
    case KeyCode::kLog:
      commitInputIfNeeded();
      performUnaryOperation(core_.commonLog(), "LOG");
      break;
    case KeyCode::kSqrt:
      commitInputIfNeeded();
      performUnaryOperation(core_.squareRoot(), "SQRT");
      break;
    case KeyCode::kSquare:
      commitInputIfNeeded();
      performUnaryOperation(core_.square(), "X^2");
      break;
    case KeyCode::kReciprocal:
      commitInputIfNeeded();
      performUnaryOperation(core_.reciprocal(), "1/X");
      break;
    case KeyCode::kPercent:
      commitInputIfNeeded();
      performUnaryOperation(core_.percent(), "%");
      break;
    case KeyCode::kPowXY:
      commitInputIfNeeded();
      performUnaryOperation(core_.power(), "X^Y");
      break;
    case KeyCode::kPi:
      if (core_.pushPi()) {
        logResult("PI");
      }
      break;
    case KeyCode::kE:
      if (core_.pushE()) {
        logResult("E");
      }
      break;
    case KeyCode::kAns:
      if (strlen(last_result_) != 0) {
        core_.push(atof(last_result_));
        logResult("ANS");
      }
      break;
    case KeyCode::kEng:
      number_base_ = NumberBase::kDec;
      break;
    case KeyCode::kDeg:
      angle_mode_ = AngleMode::kDeg;
      break;
    case KeyCode::kRad:
      angle_mode_ = AngleMode::kRad;
      break;
    case KeyCode::kBin:
      number_base_ = NumberBase::kBin;
      break;
    case KeyCode::kHex:
      number_base_ = NumberBase::kHex;
      break;
    case KeyCode::kHist:
      history_session_index_ = storage_.latestSessionIndex();
      history_view_offset_ = 0;
      screen_ = AppScreen::kHistory;
      break;
    case KeyCode::kFile:
      file_browser_session_index_ = storage_.latestSessionIndex();
      screen_ = AppScreen::kFileBrowser;
      break;
    case KeyCode::kMenu:
      screen_ = AppScreen::kMenu;
      break;
    case KeyCode::kMode:
      cycleWorkMode();
      break;
    default:
      break;
  }
}

/*
 * 函数作用：处理一次性 SHIFT 副层按键。
 * 返回值：
 * - true：本次按键已由 SHIFT 副层消费
 * - false：未定义副功能，允许继续走主层逻辑
 */
bool App::handleShiftedHomeKey(KeyCode code) {
  switch (code) {
    case KeyCode::kDel:
      clearInput();
      return true;
    case KeyCode::kHist:
      history_session_index_ = storage_.latestSessionIndex();
      history_view_offset_ = 0;
      screen_ = AppScreen::kHistory;
      return true;
    case KeyCode::kFile:
      file_browser_session_index_ = storage_.latestSessionIndex();
      screen_ = AppScreen::kFileBrowser;
      return true;
    case KeyCode::kSin:
      commitInputIfNeeded();
      performUnaryOperation(core_.arcSin(angle_mode_), "ASIN");
      return true;
    case KeyCode::kCos:
      commitInputIfNeeded();
      performUnaryOperation(core_.arcCos(angle_mode_), "ACOS");
      return true;
    case KeyCode::kTan:
      commitInputIfNeeded();
      performUnaryOperation(core_.arcTan(angle_mode_), "ATAN");
      return true;
    case KeyCode::kPi:
      if (core_.push(6.28318530718f)) {
        logResult("2PI");
      }
      return true;
    case KeyCode::kLn:
      commitInputIfNeeded();
      performUnaryOperation(core_.expE(), "EXP");
      return true;
    case KeyCode::kLog:
      commitInputIfNeeded();
      performUnaryOperation(core_.exp10(), "10^X");
      return true;
    case KeyCode::kE:
      commitInputIfNeeded();
      performUnaryOperation(core_.expE(), "E^X");
      return true;
    case KeyCode::kSqrt:
      commitInputIfNeeded();
      performUnaryOperation(core_.cubeRoot(), "CBRT");
      return true;
    case KeyCode::kSquare:
      commitInputIfNeeded();
      performUnaryOperation(core_.absoluteValue(), "ABS");
      return true;
    case KeyCode::kDrop:
      core_.clearStack();
      clearInput();
      return true;
    case KeyCode::kSto:
      commitInputIfNeeded();
      pending_register_action_ = PendingRegisterAction::kStore;
      return true;
    case KeyCode::kRcl:
      pending_register_action_ = PendingRegisterAction::kRecall;
      return true;
    case KeyCode::kDeg:
      angle_mode_ = AngleMode::kGrad;
      return true;
    case KeyCode::kRad:
      commitInputIfNeeded();
      performUnaryOperation(core_.decimalDegreesToDms(), "DMS");
      return true;
    case KeyCode::kBin:
      number_base_ = NumberBase::kOct;
      return true;
    case KeyCode::kHex:
      number_base_ = NumberBase::kDec;
      return true;
    case KeyCode::kMode:
      cycleWorkMode();
      return true;
    default:
      return false;
  }
}

/*
 * 函数作用：处理一次性 ALPHA 副层按键。
 * 说明：
 * - 当前版本主要用于程序员模式下录入十六进制 A-F。
 */
bool App::handleAlphaHomeKey(KeyCode code) {
  switch (code) {
    case KeyCode::k7:
      return appendAlphaDigit('A');
    case KeyCode::k8:
      return appendAlphaDigit('B');
    case KeyCode::k9:
      return appendAlphaDigit('C');
    case KeyCode::k4:
      return appendAlphaDigit('D');
    case KeyCode::k5:
      return appendAlphaDigit('E');
    case KeyCode::k6:
      return appendAlphaDigit('F');
    default:
      return false;
  }
}

/*
 * 函数作用：根据当前页面状态刷新 LCD。
 */
void App::renderCurrentScreen() {
  if (screen_ != AppScreen::kHome) {
    switch (screen_) {
      case AppScreen::kMenu:
        renderMenuScreen();
        return;
      case AppScreen::kHistory:
        renderHistoryScreen();
        return;
      case AppScreen::kFileBrowser:
        renderFileScreen();
        return;
      case AppScreen::kStandby:
        renderStandbyScreen();
        return;
      case AppScreen::kMaintenance:
        renderMaintenanceScreen();
        return;
      case AppScreen::kHome:
        break;
    }
  }

  AppViewModel model = {};
  strncpy(model.input_line, input_buffer_, sizeof(model.input_line) - 1);

  char top_buffer[kFormatBufferSize];
  core_.formatValue(core_.top(), number_base_, top_buffer, sizeof(top_buffer));
  snprintf(model.result_line, sizeof(model.result_line), "X=%s", top_buffer);

  buildStatusText(model.status_line, sizeof(model.status_line));
  display_.renderHome(model);
}

/*
 * 函数作用：渲染菜单界面。
 */
void App::renderMenuScreen() {
  char line1[kDisplayColumns + 1] = {};
  char line2[kDisplayColumns + 1] = {};
  const char* angle_text = angle_mode_ == AngleMode::kDeg ? "DEG" : (angle_mode_ == AngleMode::kRad ? "RAD" : "GRD");
  const char* base_text = "DEC";
  const char* mode_text = work_mode_ == WorkMode::kScientific ? "SCI" : (work_mode_ == WorkMode::kProgrammer ? "PRG" : "MNT");

  switch (number_base_) {
    case NumberBase::kDec:
      base_text = "DEC";
      break;
    case NumberBase::kBin:
      base_text = "BIN";
      break;
    case NumberBase::kOct:
      base_text = "OCT";
      break;
    case NumberBase::kHex:
      base_text = "HEX";
      break;
  }

  snprintf(
      line1,
      sizeof(line1),
      "ANG:%s BASE:%s",
      angle_text,
      base_text);
  snprintf(line2, sizeof(line2), "MODE:%s SD:%s", mode_text, storage_.available() ? "OK" : "ERR");
  display_.showMessage(line1, line2);
}

/*
 * 函数作用：从选中的历史会话中读取记录并渲染历史界面。
 */
void App::renderHistoryScreen() {
  uint16_t step = 0;
  char line1[kDisplayColumns + 1] = {};
  char line2[kDisplayColumns + 1] = {};
  char expression[kHistoryExpressionSize] = {};
  char result[kHistoryResultSize] = {};
  uint8_t available_count = 0;

  if (!storage_.readRecentRecordFromSession(
          history_session_index_,
          history_view_offset_,
          &step,
          expression,
          sizeof(expression),
          result,
          sizeof(result),
          &available_count)) {
    display_.showMessage(F("NO HISTORY"), F("BK:HOME"));
    return;
  }

  snprintf(line1, sizeof(line1), "F:%u S:%u %s", history_session_index_, step, expression);
  snprintf(line2, sizeof(line2), "R:%s %u/%u", result, history_view_offset_ + 1, available_count);
  display_.showMessage(line1, line2);
}

/*
 * 函数作用：渲染会话文件浏览界面。
 */
void App::renderFileScreen() {
  char line1[kDisplayColumns + 1] = {};
  char line2[kDisplayColumns + 1] = {};
  const uint16_t available_count = storage_.availableSessionCount();

  if (!storage_.available()) {
    display_.showMessage(F("SD NOT READY"), F("BK:HOME"));
    return;
  }

  if (available_count == 0) {
    display_.showMessage(F("LOG FILE:NONE"), F("CALC TO CREATE"));
    return;
  }

  if (file_browser_session_index_ == 0 || file_browser_session_index_ > available_count) {
    file_browser_session_index_ = available_count;
  }

  snprintf(line1, sizeof(line1), "FILE %u/%u", file_browser_session_index_, available_count);
  snprintf(line2, sizeof(line2), "%u.txt OK=OPEN", file_browser_session_index_);
  display_.showMessage(line1, line2);
}

/*
 * 函数作用：渲染待机界面。
 */
void App::renderStandbyScreen() {
  display_.showMessage(F("SYSTEM STANDBY"), F("ON=RESUME"));
}

/*
 * 函数作用：渲染维护/自检界面。
 */
void App::renderMaintenanceScreen() {
  char line1[kDisplayColumns + 1] = {};
  char line2[kDisplayColumns + 1] = {};

  snprintf(line1, sizeof(line1), "MNT SD:%s FILE:%u", storage_.available() ? "OK" : "ER", storage_.availableSessionCount());
  snprintf(line2, sizeof(line2), "HIST/FILE BK:EXIT");
  display_.showMessage(line1, line2);
}

/*
 * 函数作用：向当前输入缓冲区追加一个字符。
 */
void App::appendInputChar(char ch) {
  if (input_length_ + 1 >= kInputBufferSize) {
    return;
  }
  input_buffer_[input_length_++] = ch;
  input_buffer_[input_length_] = '\0';
}

/*
 * 函数作用：清空当前输入缓冲区。
 */
void App::clearInput() {
  input_length_ = 0;
  input_buffer_[0] = '\0';
}

/*
 * 函数作用：若输入缓冲区非空，则把文本转换为数值并压栈。
 */
bool App::commitInputIfNeeded() {
  if (input_length_ == 0) {
    return false;
  }

  float value = 0.0f;
  if (!parseInputValue(&value)) {
    return false;
  }
  core_.push(value);
  clearInput();
  return true;
}

/*
 * 函数作用：按当前显示进制解析输入缓冲区。
 */
bool App::parseInputValue(float* out_value) const {
  if (out_value == nullptr || input_length_ == 0) {
    return false;
  }

  char* end_ptr = nullptr;

  if (number_base_ == NumberBase::kDec) {
    *out_value = static_cast<float>(strtod(input_buffer_, &end_ptr));
  } else if (number_base_ == NumberBase::kBin) {
    const long value = strtol(input_buffer_, &end_ptr, 2);
    *out_value = static_cast<float>(value);
  } else if (number_base_ == NumberBase::kOct) {
    const long value = strtol(input_buffer_, &end_ptr, 8);
    *out_value = static_cast<float>(value);
  } else {
    const long value = strtol(input_buffer_, &end_ptr, 16);
    *out_value = static_cast<float>(value);
  }

  return end_ptr != nullptr && *end_ptr == '\0';
}

/*
 * 函数作用：在程序员模式下追加一个十六进制字符。
 */
bool App::appendAlphaDigit(char ch) {
  if (number_base_ != NumberBase::kHex) {
    return false;
  }

  appendInputChar(ch);
  return true;
}

/*
 * 函数作用：清空一次性副层状态。
 */
void App::clearModifierArms() {
  shift_armed_ = false;
  alpha_armed_ = false;
}

/*
 * 函数作用：在寄存器待选状态下，用数字键完成 STO/RCL 槽位操作。
 */
bool App::handlePendingRegisterDigit(KeyCode code) {
  uint8_t slot_index = 0;

  switch (code) {
    case KeyCode::k0: slot_index = 0; break;
    case KeyCode::k1: slot_index = 1; break;
    case KeyCode::k2: slot_index = 2; break;
    case KeyCode::k3: slot_index = 3; break;
    case KeyCode::k4: slot_index = 4; break;
    case KeyCode::k5: slot_index = 5; break;
    case KeyCode::k6: slot_index = 6; break;
    case KeyCode::k7: slot_index = 7; break;
    case KeyCode::k8: slot_index = 8; break;
    case KeyCode::k9: slot_index = 9; break;
    default:
      return false;
  }

  if (pending_register_action_ == PendingRegisterAction::kStore) {
    commitInputIfNeeded();
    core_.storeSlot(slot_index);
    logResult("STO SLOT");
  } else if (pending_register_action_ == PendingRegisterAction::kRecall) {
    if (core_.recallSlot(slot_index)) {
      logResult("RCL SLOT");
    }
  }

  pending_register_action_ = PendingRegisterAction::kNone;
  return true;
}

/*
 * 函数作用：执行二元运算并在成功后记录历史。
 */
void App::performBinaryOperation(BinaryOp op, const char* expression_text) {
  commitInputIfNeeded();
  if (core_.applyBinary(op)) {
    logResult(expression_text);
  }
}

/*
 * 函数作用：执行一元运算并在成功后记录历史。
 */
void App::performUnaryOperation(bool succeeded, const char* expression_text) {
  if (!succeeded) {
    return;
  }
  logResult(expression_text);
}

/*
 * 函数作用：把当前计算结果写入运行态缓存，并追加到 SD 历史文件。
 */
void App::logResult(const char* expression_text) {
  char result_text[kResultBufferSize];
  core_.formatValue(core_.top(), NumberBase::kDec, result_text, sizeof(result_text));
  strncpy(last_result_, result_text, sizeof(last_result_) - 1);
  last_result_[sizeof(last_result_) - 1] = '\0';

  const uint16_t step = ++step_counter_;

  if (storage_.available()) {
    HistoryRecord record = {};
    record.step = step;
    record.expression = expression_text;
    record.result = result_text;

    if (!storage_.appendRecord(record)) {
      // 存储失败时保持计算链路继续工作，由状态栏反映 SD 状态。
    }
  }
}

/*
 * 函数作用：构造主界面底部状态行。
 */
void App::buildStatusText(char* out_text, size_t length) const {
  const char* angle_text = angle_mode_ == AngleMode::kDeg ? "DEG" : (angle_mode_ == AngleMode::kRad ? "RAD" : "GRD");
  const char* base_text = "DEC";
  const char* error_text = diagnostics_.errorText(core_.last_error());
  const char* mode_text = work_mode_ == WorkMode::kScientific ? "SCI" : (work_mode_ == WorkMode::kProgrammer ? "PRG" : "MNT");
  const char* storage_text = storage_.available() ? "SD" : "ER";
  char modifier_text[6] = {};

  switch (number_base_) {
    case NumberBase::kDec:
      base_text = "DEC";
      break;
    case NumberBase::kBin:
      base_text = "BIN";
      break;
    case NumberBase::kOct:
      base_text = "OCT";
      break;
    case NumberBase::kHex:
      base_text = "HEX";
      break;
  }

  if (pending_register_action_ == PendingRegisterAction::kStore) {
    strncpy(modifier_text, " ST", sizeof(modifier_text) - 1);
  } else if (pending_register_action_ == PendingRegisterAction::kRecall) {
    strncpy(modifier_text, " RC", sizeof(modifier_text) - 1);
  } else if (shift_armed_) {
    strncpy(modifier_text, " S", sizeof(modifier_text) - 1);
  } else if (alpha_armed_) {
    strncpy(modifier_text, " A", sizeof(modifier_text) - 1);
  }

  snprintf(
      out_text,
      length,
      "%s %s %s %s%s",
      mode_text,
      angle_text,
      core_.last_error() == ErrorCode::kNone ? base_text : error_text,
      core_.last_error() == ErrorCode::kNone ? storage_text : "",
      modifier_text);
}

/*
 * 函数作用：按固定顺序切换工作模式，并同步页面。
 */
void App::cycleWorkMode() {
  switch (work_mode_) {
    case WorkMode::kScientific:
      work_mode_ = WorkMode::kProgrammer;
      number_base_ = NumberBase::kHex;
      screen_ = AppScreen::kHome;
      break;
    case WorkMode::kProgrammer:
      work_mode_ = WorkMode::kMaintenance;
      screen_ = AppScreen::kMaintenance;
      break;
    case WorkMode::kMaintenance:
      work_mode_ = WorkMode::kScientific;
      angle_mode_ = AngleMode::kDeg;
      number_base_ = NumberBase::kDec;
      screen_ = AppScreen::kHome;
      break;
  }
}

}  // namespace calculator
