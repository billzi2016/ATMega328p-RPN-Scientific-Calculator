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
  /*
   * 函数作用：初始化 SD 模块与历史目录管理器。
   */
  void begin();

  /*
   * 函数作用：返回 SD 当前是否可用。
   */
  bool available() const;

  /*
   * 函数作用：确保本次会话日志文件已经创建并打开。
   */
  bool ensureSessionFile();

  /*
   * 函数作用：向当前会话文件追加一条历史记录。
   */
  bool appendRecord(const HistoryRecord& record);

  /*
   * 函数作用：返回本次会话日志文件是否已经真正创建。
   */
  bool sessionOpened() const;

  /*
   * 函数作用：返回当前会话日志文件路径。
   */
  const char* sessionPath() const;

  /*
   * 函数作用：返回当前可浏览的历史会话总数。
   */
  uint16_t availableSessionCount() const;

  /*
   * 函数作用：返回当前最新会话编号。
   */
  uint16_t latestSessionIndex() const;

  /*
   * 函数作用：返回当前已打开会话的编号，若尚未创建则返回 0。
   */
  uint16_t currentSessionIndex() const;

  /*
   * 函数作用：从当前会话日志文件中读取最近的第 N 条记录。
   * 参数说明：
   * - offset=0 表示最新一条
   * - out_available_count 返回当前可浏览的最近记录条数
   */
  bool readRecentRecord(
      uint8_t offset,
      uint16_t* out_step,
      char* out_expression,
      size_t expression_length,
      char* out_result,
      size_t result_length,
      uint8_t* out_available_count) const;

  /*
   * 函数作用：从指定会话文件中读取最近的第 N 条记录。
   */
  bool readRecentRecordFromSession(
      uint16_t session_index,
      uint8_t offset,
      uint16_t* out_step,
      char* out_expression,
      size_t expression_length,
      char* out_result,
      size_t result_length,
      uint8_t* out_available_count) const;

 private:
  bool ready_ = false;
  bool session_opened_ = false;
  char session_path_[kPathBufferSize] = {};
  File session_file_;
  HistoryManager history_manager_;

  bool openSessionFileIfNeeded();
  bool readRecentRecordFromPath(
      const char* path,
      uint8_t offset,
      uint16_t* out_step,
      char* out_expression,
      size_t expression_length,
      char* out_result,
      size_t result_length,
      uint8_t* out_available_count) const;

  /*
   * 函数作用：把单行 STEP 日志解析为步号、表达式和结果。
   */
  bool parseHistoryLine(
      const char* line,
      uint16_t* out_step,
      char* out_expression,
      size_t expression_length,
      char* out_result,
      size_t result_length) const;
};

}  // namespace calculator

#endif  // STORAGE_SD_H
