/*
 * 文件名称：diagnostics.h
 * 文件作用：声明诊断与错误报告接口。
 * 设计意图：
 * 1. 为上电自检、错误码转字符串、后续串口诊断预留统一出口。
 * 2. 避免把错误文本散落在多个业务模块里。
 */

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <Arduino.h>

#include "types.h"

namespace calculator {

class Diagnostics {
 public:
  void begin();
  const char* errorText(ErrorCode code) const;
};

}  // namespace calculator

#endif  // DIAGNOSTICS_H
