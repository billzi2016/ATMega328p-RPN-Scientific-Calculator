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
  /*
   * 函数作用：扫描现有日志目录并定位下一个可用会话编号。
   */
  bool begin();

  /*
   * 函数作用：构造下一个新会话文件路径。
   */
  bool buildNextSessionPath(char* out_path, size_t length);

  /*
   * 函数作用：按指定会话编号构造固定路径。
   */
  bool buildSessionPathForIndex(uint16_t session_index, char* out_path, size_t length) const;

  /*
   * 函数作用：返回当前已经存在的历史会话数量。
   */
  uint16_t existingSessionCount() const;

  /*
   * 函数作用：返回下一个待分配的会话编号。
   */
  uint16_t nextSessionIndex() const;

 private:
  uint16_t next_session_index_ = 1;
  bool ensureLogDirectory();
};

}  // namespace calculator

#endif  // HISTORY_MANAGER_H
