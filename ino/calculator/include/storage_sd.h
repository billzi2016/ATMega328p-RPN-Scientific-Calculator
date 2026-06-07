/*
 * 文件名称：storage_sd.h
 * 文件作用：声明 TF/SD 历史存储模块接口。
 * 设计意图：
 * 1. 封装 SD 初始化、会话文件创建、历史记录追加。
 * 2. 当 SD 不可用时，对上层提供可降级判断，而不是让系统整体失败。
 */

#ifndef STORAGE_SD_H
#define STORAGE_SD_H

#include <Arduino.h>
#include <SD.h>

#include "history_manager.h"
#include "types.h"

namespace calculator {

class StorageSd {
 public:
  void begin();
  bool available() const;
  bool ensureSessionFile();
  bool appendRecord(const HistoryRecord& record);
  bool sessionOpened() const;
  const char* sessionPath() const;

 private:
  bool ready_ = false;
  bool session_opened_ = false;
  char session_path_[kPathBufferSize] = {};
  File session_file_;
  HistoryManager history_manager_;

  bool openSessionFileIfNeeded();
};

}  // namespace calculator

#endif  // STORAGE_SD_H
