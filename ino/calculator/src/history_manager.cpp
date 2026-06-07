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

bool HistoryManager::begin() {
  if (!ensureLogDirectory()) {
    return false;
  }

  next_session_index_ = 1;
  char path[kPathBufferSize];

  while (true) {
    snprintf(path, sizeof(path), "%s/%u.txt", kLogDirectoryName, next_session_index_);
    if (!SD.exists(path)) {
      return true;
    }
    ++next_session_index_;
  }
}

bool HistoryManager::buildNextSessionPath(char* out_path, size_t length) {
  if (out_path == nullptr || length == 0) {
    return false;
  }

  snprintf(out_path, length, "%s/%u.txt", kLogDirectoryName, next_session_index_);
  return true;
}

bool HistoryManager::ensureLogDirectory() {
  if (SD.exists(kLogDirectoryName)) {
    return true;
  }
  return SD.mkdir(kLogDirectoryName);
}

}  // namespace calculator
