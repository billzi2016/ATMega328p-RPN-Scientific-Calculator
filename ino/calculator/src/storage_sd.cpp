/*
 * 文件名称：storage_sd.cpp
 * 文件作用：实现 TF/SD 历史记录存储。
 * 设计意图：
 * 1. 以“本次开机第一次成功计算后创建一个会话文件”为策略落地 PRD。
 * 2. 当存储不可用时允许系统降级运行，不阻塞计算功能。
 */

#include "../include/storage_sd.h"

#include <SPI.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/config.h"
#include "../include/pins.h"

namespace calculator {

/*
 * 函数作用：初始化 SD 卡并准备历史目录管理器。
 * 说明：
 * - 若 SD 初始化失败，系统仍允许继续计算，只是历史功能降级。
 */
void StorageSd::begin() {
  ready_ = SD.begin(kSdCsPin);
  session_opened_ = false;

  if (!ready_) {
    return;
  }

  ready_ = history_manager_.begin();
}

bool StorageSd::available() const {
  return ready_;
}

/*
 * 函数作用：确保当前会话文件已创建。
 */
bool StorageSd::ensureSessionFile() {
  return openSessionFileIfNeeded();
}

bool StorageSd::sessionOpened() const {
  return session_opened_;
}

const char* StorageSd::sessionPath() const {
  return session_path_;
}

/*
 * 函数作用：从当前会话文件中读取最近的若干条记录之一。
 * 说明：
 * - 这里每次进入历史页都直接重新读 SD，不依赖 RAM 中的多条缓存。
 * - offset=0 代表最新一条，offset 增大表示更早的记录。
 */
bool StorageSd::readRecentRecord(
    uint8_t offset,
    uint16_t* out_step,
    char* out_expression,
    size_t expression_length,
    char* out_result,
    size_t result_length,
    uint8_t* out_available_count) const {
  if (out_step != nullptr) {
    *out_step = 0;
  }
  if (out_expression != nullptr && expression_length > 0) {
    out_expression[0] = '\0';
  }
  if (out_result != nullptr && result_length > 0) {
    out_result[0] = '\0';
  }
  if (out_available_count != nullptr) {
    *out_available_count = 0;
  }

  if (!ready_ || !session_opened_) {
    return false;
  }

  File read_file = SD.open(session_path_, FILE_READ);
  if (!read_file) {
    return false;
  }

  uint16_t recent_steps[kHistoryCapacity] = {};
  char recent_expressions[kHistoryCapacity][kHistoryExpressionSize] = {};
  char recent_results[kHistoryCapacity][kHistoryResultSize] = {};
  uint8_t recent_count = 0;
  uint8_t recent_head = 0;
  char line_buffer[64] = {};
  size_t line_length = 0;

  while (read_file.available()) {
    const char incoming = static_cast<char>(read_file.read());
    if (incoming == '\r') {
      continue;
    }

    if (incoming != '\n' && line_length + 1 < sizeof(line_buffer)) {
      line_buffer[line_length++] = incoming;
      line_buffer[line_length] = '\0';
      continue;
    }

    if (line_length != 0) {
      uint16_t step = 0;
      char expression[kHistoryExpressionSize] = {};
      char result[kHistoryResultSize] = {};
      if (parseHistoryLine(line_buffer, &step, expression, sizeof(expression), result, sizeof(result))) {
        recent_steps[recent_head] = step;
        strncpy(recent_expressions[recent_head], expression, sizeof(recent_expressions[recent_head]) - 1);
        strncpy(recent_results[recent_head], result, sizeof(recent_results[recent_head]) - 1);
        recent_head = static_cast<uint8_t>((recent_head + 1) % kHistoryCapacity);
        if (recent_count < kHistoryCapacity) {
          ++recent_count;
        }
      }
    }

    line_length = 0;
    line_buffer[0] = '\0';
  }

  if (line_length != 0) {
    uint16_t step = 0;
    char expression[kHistoryExpressionSize] = {};
    char result[kHistoryResultSize] = {};
    if (parseHistoryLine(line_buffer, &step, expression, sizeof(expression), result, sizeof(result))) {
      recent_steps[recent_head] = step;
      strncpy(recent_expressions[recent_head], expression, sizeof(recent_expressions[recent_head]) - 1);
      strncpy(recent_results[recent_head], result, sizeof(recent_results[recent_head]) - 1);
      recent_head = static_cast<uint8_t>((recent_head + 1) % kHistoryCapacity);
      if (recent_count < kHistoryCapacity) {
        ++recent_count;
      }
    }
  }

  read_file.close();

  if (out_available_count != nullptr) {
    *out_available_count = recent_count;
  }
  if (offset >= recent_count) {
    return false;
  }

  int16_t target_index = static_cast<int16_t>(recent_head) - 1 - offset;
  while (target_index < 0) {
    target_index += kHistoryCapacity;
  }

  if (out_step != nullptr) {
    *out_step = recent_steps[target_index];
  }
  if (out_expression != nullptr && expression_length > 0) {
    strncpy(out_expression, recent_expressions[target_index], expression_length - 1);
    out_expression[expression_length - 1] = '\0';
  }
  if (out_result != nullptr && result_length > 0) {
    strncpy(out_result, recent_results[target_index], result_length - 1);
    out_result[result_length - 1] = '\0';
  }
  return true;
}

bool StorageSd::appendRecord(const HistoryRecord& record) {
  if (!openSessionFileIfNeeded()) {
    return false;
  }

  session_file_.print(F("STEP="));
  session_file_.print(record.step);
  session_file_.print(F(" | EXPR="));
  session_file_.print(record.expression == nullptr ? "" : record.expression);
  session_file_.print(F(" | RESULT="));
  session_file_.println(record.result == nullptr ? "" : record.result);
  session_file_.flush();
  return true;
}

bool StorageSd::openSessionFileIfNeeded() {
  if (!ready_) {
    return false;
  }

  if (session_opened_) {
    return true;
  }

  char path[kPathBufferSize];
  if (!history_manager_.buildNextSessionPath(path, sizeof(path))) {
    return false;
  }

  session_file_ = SD.open(path, FILE_WRITE);
  if (!session_file_) {
    ready_ = false;
    return false;
  }

  session_file_.println(F("MODE=RPN"));
  session_file_.println(F("ANGLE=DEG"));
  session_file_.println(F("----"));
  session_file_.flush();
  strncpy(session_path_, path, sizeof(session_path_) - 1);
  session_path_[sizeof(session_path_) - 1] = '\0';
  session_opened_ = true;
  return true;
}

/*
 * 函数作用：解析单行历史记录文本。
 * 输入格式：
 * STEP=0001 | EXPR=... | RESULT=...
 */
bool StorageSd::parseHistoryLine(
    const char* line,
    uint16_t* out_step,
    char* out_expression,
    size_t expression_length,
    char* out_result,
    size_t result_length) const {
  if (line == nullptr || strncmp(line, "STEP=", 5) != 0) {
    return false;
  }

  const char* expr_tag = strstr(line, " | EXPR=");
  const char* result_tag = strstr(line, " | RESULT=");
  if (expr_tag == nullptr || result_tag == nullptr || expr_tag >= result_tag) {
    return false;
  }

  if (out_step != nullptr) {
    *out_step = static_cast<uint16_t>(atoi(line + 5));
  }

  const char* expr_start = expr_tag + 8;
  const size_t expr_size = static_cast<size_t>(result_tag - expr_start);
  if (out_expression != nullptr && expression_length > 0) {
    const size_t copy_size = expr_size < (expression_length - 1) ? expr_size : (expression_length - 1);
    memcpy(out_expression, expr_start, copy_size);
    out_expression[copy_size] = '\0';
  }

  const char* result_start = result_tag + 10;
  if (out_result != nullptr && result_length > 0) {
    strncpy(out_result, result_start, result_length - 1);
    out_result[result_length - 1] = '\0';
  }

  return true;
}

}  // namespace calculator
