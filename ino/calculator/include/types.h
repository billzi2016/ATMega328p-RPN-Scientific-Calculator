/*
 * 文件名称：types.h
 * 文件作用：集中定义跨模块共享的枚举、结构体和通用类型。
 * 设计意图：
 * 1. 让不同模块通过统一类型通信，避免魔法数字四处扩散。
 * 2. 为后续扩展更多模式、键值、错误码预留稳定接口。
 */

#ifndef CALCULATOR_TYPES_H
#define CALCULATOR_TYPES_H

#include <Arduino.h>

namespace calculator {

enum class AppScreen : uint8_t {
  kHome = 0,
  kMenu,
  kHistory,
  kFileBrowser,
  kStandby,
  kMaintenance
};

enum class AngleMode : uint8_t {
  kDeg = 0,
  kRad,
  kGrad
};

enum class NumberBase : uint8_t {
  kDec = 0,
  kBin,
  kOct,
  kHex
};

enum class WorkMode : uint8_t {
  kScientific = 0,
  kProgrammer,
  kMaintenance
};

enum class ErrorCode : uint8_t {
  kNone = 0,
  kDivideByZero,
  kOverflow,
  kStackUnderflow,
  kStackOverflow,
  kStorageError
};

enum class BinaryOp : uint8_t {
  kAdd = 0,
  kSubtract,
  kMultiply,
  kDivide
};

enum class KeyCode : uint8_t {
  kNone = 0,
  kOn, kOff, kMenu, kMode, kHist, kFile, kUp, kDel,
  kShift, kAlpha, kLeft, kOk, kRight, kLParen, kRParen, kBack,
  k7, k8, k9, kDivide, kSin, kCos, kTan, kPi,
  k4, k5, k6, kMultiply, kLn, kLog, kPowXY, kE,
  k1, k2, k3, kMinus, kSqrt, kSquare, kReciprocal, kPercent,
  k0, kDot, kSign, kPlus, kExp, kEng, kAns, kEqual,
  kEnter, kSwap, kDrop, kDup, kRollUp, kRollDown, kSto, kRcl,
  kMc, kMr, kMPlus, kMMinus, kDeg, kRad, kBin, kHex
};

struct KeyEvent {
  KeyCode code;
  bool pressed;
  uint32_t timestamp_ms;
};

struct HistoryRecord {
  uint16_t step;
  const char* expression;
  const char* result;
};

struct AppViewModel {
  char input_line[21];
  char result_line[21];
  char status_line[21];
};

}  // namespace calculator

#endif  // CALCULATOR_TYPES_H
