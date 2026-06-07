/*
 * 文件名称：history_manager.h
 * 文件作用：声明历史文件编号与路径管理接口。
 * 设计意图：
 * 1. 把 1.txt / 2.txt / 3.txt 顺延策略集中管理。
 * 2. 避免 SD 存储模块同时承担目录扫描和日志写入两种职责。
 */

#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <Arduino.h>

#include "config.h"

namespace calculator {

class HistoryManager {
 public:
  bool begin();
  bool buildNextSessionPath(char* out_path, size_t length);

 private:
  uint16_t next_session_index_ = 1;
  bool ensureLogDirectory();
};

}  // namespace calculator

#endif  // HISTORY_MANAGER_H
