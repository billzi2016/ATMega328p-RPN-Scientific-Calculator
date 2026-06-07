/*
 * 文件名称：config.h
 * 文件作用：集中定义系统配置常量。
 * 设计意图：
 * 1. 统一维护缓冲区长度、扫描周期、栈深、文件路径等基础参数。
 * 2. 让后续调优时只改一个地方，避免散落常量造成维护混乱。
 */

#ifndef CALCULATOR_CONFIG_H
#define CALCULATOR_CONFIG_H

#include <Arduino.h>

namespace calculator {

constexpr uint8_t kKeyboardRowCount = 8;
constexpr uint8_t kKeyboardColumnCount = 8;
constexpr uint16_t kKeyboardDebounceMs = 30;
constexpr uint16_t kKeyboardScanIntervalMs = 10;

constexpr uint8_t kStackDepth = 4;
constexpr size_t kInputBufferSize = 24;
constexpr size_t kFormatBufferSize = 20;

constexpr uint16_t kDisplayColumns = 20;
constexpr uint16_t kDisplayRows = 4;

constexpr char kLogDirectoryName[] = "LOGS";
constexpr size_t kPathBufferSize = 24;
constexpr size_t kResultBufferSize = 20;

constexpr uint16_t kMainLoopDelayMs = 5;

}  // namespace calculator

#endif  // CALCULATOR_CONFIG_H
