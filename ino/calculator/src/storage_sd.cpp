/*
 * 文件名称：storage_sd.cpp
 * 文件作用：实现 TF/SD 历史记录存储。
 * 设计意图：
 * 1. 以“本次开机第一次成功计算后创建一个会话文件”为策略落地 PRD。
 * 2. 当存储不可用时允许系统降级运行，不阻塞计算功能。
 */

#include "../include/storage_sd.h"

#include <SPI.h>
#include <stdio.h>

#include "../include/config.h"
#include "../include/pins.h"

namespace calculator {

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

bool StorageSd::ensureSessionFile() {
  return openSessionFileIfNeeded();
}

bool StorageSd::sessionOpened() const {
  return session_opened_;
}

const char* StorageSd::sessionPath() const {
  return session_path_;
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

}  // namespace calculator
