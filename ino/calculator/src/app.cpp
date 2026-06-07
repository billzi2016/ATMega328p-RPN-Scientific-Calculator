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

  switch (screen_) {
    case AppScreen::kHome:
      handleHomeKey(event.code);
      break;
    case AppScreen::kMenu:
      if (event.code == KeyCode::kBack || event.code == KeyCode::kMenu || event.code == KeyCode::kMode) {
        screen_ = AppScreen::kHome;
      } else if (event.code == KeyCode::kDeg) {
        angle_mode_ = AngleMode::kDeg;
      } else if (event.code == KeyCode::kRad) {
        angle_mode_ = AngleMode::kRad;
      } else if (event.code == KeyCode::kBin) {
        number_base_ = NumberBase::kBin;
      } else if (event.code == KeyCode::kHex) {
        number_base_ = NumberBase::kHex;
      } else if (event.code == KeyCode::kAns) {
        number_base_ = NumberBase::kDec;
      }
      break;
    case AppScreen::kHistory:
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
      } else if (event.code == KeyCode::kDel && history_view_offset_ > 0) {
        --history_view_offset_;
      } else if (event.code == KeyCode::kLeft && history_session_index_ > 1) {
        --history_session_index_;
        history_view_offset_ = 0;
      } else if (event.code == KeyCode::kRight && history_session_index_ < storage_.availableSessionCount()) {
        ++history_session_index_;
        history_view_offset_ = 0;
      } else if (event.code == KeyCode::kOk) {
        file_browser_session_index_ = history_session_index_;
        screen_ = AppScreen::kFileBrowser;
      }
      break;
    case AppScreen::kFileBrowser:
      if (event.code == KeyCode::kBack || event.code == KeyCode::kFile) {
        screen_ = AppScreen::kHome;
      } else if (event.code == KeyCode::kLeft && file_browser_session_index_ > 1) {
        --file_browser_session_index_;
      } else if (event.code == KeyCode::kRight && file_browser_session_index_ < storage_.availableSessionCount()) {
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
}

/*
 * 函数作用：处理主计算界面的按键逻辑。
 */
void App::handleHomeKey(KeyCode code) {
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
      screen_ = AppScreen::kHome;
      break;
    case KeyCode::kOff:
      core_.reset();
      clearInput();
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
    case KeyCode::kMode:
      screen_ = AppScreen::kMenu;
      break;
    default:
      break;
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

  snprintf(
      line1,
      sizeof(line1),
      "ANG:%s BASE:%s",
      angle_mode_ == AngleMode::kDeg ? "DEG" : "RAD",
      number_base_ == NumberBase::kBin ? "BIN" : (number_base_ == NumberBase::kHex ? "HEX" : "DEC"));
  snprintf(line2, sizeof(line2), "SD:%s BK:EXIT", storage_.available() ? "OK" : "ERR");
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

  const float value = atof(input_buffer_);
  core_.push(value);
  clearInput();
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
  const char* angle_text = angle_mode_ == AngleMode::kDeg ? "DEG" : "RAD";
  const char* base_text = "DEC";
  const char* error_text = diagnostics_.errorText(core_.last_error());

  switch (number_base_) {
    case NumberBase::kDec:
      base_text = "DEC";
      break;
    case NumberBase::kBin:
      base_text = "BIN";
      break;
    case NumberBase::kHex:
      base_text = "HEX";
      break;
  }

  snprintf(
      out_text,
      length,
      "%s %s %s",
      angle_text,
      base_text,
      core_.last_error() == ErrorCode::kNone ? (storage_.available() ? "SD:OK" : "SD:ER") : error_text);
}

}  // namespace calculator
