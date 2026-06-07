/*
 * 文件名称：history_manager.cpp
 * 文件作用：实现历史文件编号管理。
 * 设计意图：
 * 1. 在 SD 卡中扫描现有日志，生成下一个可用的会话文件名。
 * 2. 把顺延命名策略与日志写入动作解耦，便于后续调整存储规则。
 */

#include "../include/history_manager.h"

#include <SD.h>
#include <stdio.h>

namespace calculator {

/*
 * 函数作用：扫描日志目录，确定下一个可用会话编号。
 */
bool HistoryManager::begin() {
  if (!ensureLogDirectory()) {
    return false;
  }

  next_session_index_ = 1;
  char path[kPathBufferSize];

  while (true) {
    if (!buildSessionPathForIndex(next_session_index_, path, sizeof(path))) {
      return false;
    }
    if (!SD.exists(path)) {
      return true;
    }
    ++next_session_index_;
  }
}

/*
 * 函数作用：构造下一个新会话文件路径。
 */
bool HistoryManager::buildNextSessionPath(char* out_path, size_t length) {
  return buildSessionPathForIndex(next_session_index_, out_path, length);
}

/*
 * 函数作用：按指定编号构造会话文件路径。
 */
bool HistoryManager::buildSessionPathForIndex(uint16_t session_index, char* out_path, size_t length) const {
  if (out_path == nullptr || length == 0 || session_index == 0) {
    return false;
  }

  snprintf(out_path, length, "%s/%u.txt", kLogDirectoryName, session_index);
  return true;
}

/*
 * 函数作用：返回当前已经存在的历史会话数量。
 */
uint16_t HistoryManager::existingSessionCount() const {
  return static_cast<uint16_t>(next_session_index_ == 0 ? 0 : next_session_index_ - 1);
}

/*
 * 函数作用：返回下一个待分配的会话编号。
 */
uint16_t HistoryManager::nextSessionIndex() const {
  return next_session_index_;
}

/*
 * 函数作用：确保日志目录存在。
 */
bool HistoryManager::ensureLogDirectory() {
  if (SD.exists(kLogDirectoryName)) {
    return true;
  }
  return SD.mkdir(kLogDirectoryName);
}

}  // namespace calculator
