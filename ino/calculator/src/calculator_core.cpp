/*
 * 文件名称：calculator_core.cpp
 * 文件作用：实现 RPN 计算核心。
 * 设计意图：
 * 1. 当前先实现最基础的四级栈与四则运算，保证工程骨架先能跑通。
 * 2. 后续可在这个模块继续扩展科学函数，而不影响界面和存储层。
 */

#include "../include/calculator_core.h"

#include <math.h>
#include <string.h>

namespace calculator {

void CalculatorCore::begin() {
  reset();
}

void CalculatorCore::reset() {
  for (uint8_t i = 0; i < kStackDepth; ++i) {
    stack_[i] = 0.0f;
  }
  last_error_ = ErrorCode::kNone;
}

bool CalculatorCore::push(float value) {
  // 这里采用固定四级栈模型：新值进入 X，原有 X/Y/Z 依次上推。
  for (int8_t i = kStackDepth - 1; i > 0; --i) {
    stack_[i] = stack_[i - 1];
  }
  stack_[0] = value;
  last_error_ = ErrorCode::kNone;
  return true;
}

bool CalculatorCore::enter() {
  return push(stack_[0]);
}

bool CalculatorCore::drop() {
  shiftDownFrom(0);
  stack_[kStackDepth - 1] = 0.0f;
  last_error_ = ErrorCode::kNone;
  return true;
}

bool CalculatorCore::swap() {
  const float temp = stack_[0];
  stack_[0] = stack_[1];
  stack_[1] = temp;
  last_error_ = ErrorCode::kNone;
  return true;
}

bool CalculatorCore::dup() {
  return push(stack_[0]);
}

bool CalculatorCore::applyBinary(BinaryOp op) {
  const float x = stack_[0];
  const float y = stack_[1];
  float result = 0.0f;

  switch (op) {
    case BinaryOp::kAdd:
      result = y + x;
      break;
    case BinaryOp::kSubtract:
      result = y - x;
      break;
    case BinaryOp::kMultiply:
      result = y * x;
      break;
    case BinaryOp::kDivide:
      if (fabs(x) < 0.000001f) {
        setError(ErrorCode::kDivideByZero);
        return false;
      }
      result = y / x;
      break;
  }

  stack_[1] = result;
  shiftDownFrom(0);
  stack_[kStackDepth - 1] = 0.0f;
  last_error_ = ErrorCode::kNone;
  return true;
}

bool CalculatorCore::toggleSign() {
  stack_[0] = -stack_[0];
  last_error_ = ErrorCode::kNone;
  return true;
}

float CalculatorCore::top() const {
  return stack_[0];
}

void CalculatorCore::formatTop(char* buffer, size_t length) const {
  if (buffer == nullptr || length == 0) {
    return;
  }

  dtostrf(stack_[0], 0, 6, buffer);

  // 去掉尾部多余的空格，避免 LCD 上显示得松散。
  for (size_t i = 0; i < length && buffer[i] != '\0'; ++i) {
    if (buffer[i] == ' ') {
      memmove(&buffer[i], &buffer[i + 1], strlen(&buffer[i + 1]) + 1);
      --i;
    }
  }
}

ErrorCode CalculatorCore::last_error() const {
  return last_error_;
}

void CalculatorCore::setError(ErrorCode code) {
  last_error_ = code;
}

void CalculatorCore::shiftDownFrom(uint8_t index) {
  for (uint8_t i = index; i + 1 < kStackDepth; ++i) {
    stack_[i] = stack_[i + 1];
  }
}

}  // namespace calculator
