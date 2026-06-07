/*
 * 文件名称：diagnostics.cpp
 * 文件作用：实现诊断与错误文本转换。
 * 设计意图：
 * 1. 统一管理错误码到短文本的映射，便于 LCD 显示和后续日志扩展。
 * 2. 当前先保持实现简单，为后续串口自检或维护模式留口子。
 */

#include "../include/diagnostics.h"

namespace calculator {

void Diagnostics::begin() {
  // 当前版本暂不启用串口调试，保留空实现作为后续扩展入口。
}

const char* Diagnostics::errorText(ErrorCode code) const {
  switch (code) {
    case ErrorCode::kNone:
      return "OK";
    case ErrorCode::kDivideByZero:
      return "DIV0";
    case ErrorCode::kOverflow:
      return "OVERFLOW";
    case ErrorCode::kStackUnderflow:
      return "STACK ERR";
    case ErrorCode::kStackOverflow:
      return "STACK FULL";
    case ErrorCode::kStorageError:
      return "SD ERR";
  }
  return "UNKNOWN";
}

}  // namespace calculator
