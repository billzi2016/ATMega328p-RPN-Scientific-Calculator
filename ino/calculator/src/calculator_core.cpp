/*
 * 文件名称：calculator_core.cpp
 * 文件作用：实现 RPN 计算核心。
 * 设计意图：
 * 1. 提供稳定的四级栈、基础运算、科学函数和寄存器功能。
 * 2. 把数值处理与界面层解耦，便于后续继续扩展更多模式。
 */

#include "../include/calculator_core.h"

#include <ctype.h>
#include <math.h>
#include <string.h>

namespace calculator {

namespace {

constexpr float kPiValue = 3.14159265358979323846f;

}  // namespace

void CalculatorCore::begin() {
  reset();
}

void CalculatorCore::reset() {
  for (uint8_t i = 0; i < kStackDepth; ++i) {
    stack_[i] = 0.0f;
  }
  storage_register_ = 0.0f;
  memory_register_ = 0.0f;
  last_error_ = ErrorCode::kNone;
}

bool CalculatorCore::push(float value) {
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

bool CalculatorCore::rollUp() {
  const float bottom = stack_[kStackDepth - 1];
  for (int8_t i = kStackDepth - 1; i > 0; --i) {
    stack_[i] = stack_[i - 1];
  }
  stack_[0] = bottom;
  last_error_ = ErrorCode::kNone;
  return true;
}

bool CalculatorCore::rollDown() {
  const float top = stack_[0];
  for (uint8_t i = 0; i + 1 < kStackDepth; ++i) {
    stack_[i] = stack_[i + 1];
  }
  stack_[kStackDepth - 1] = top;
  last_error_ = ErrorCode::kNone;
  return true;
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

  if (!requireFinite(result)) {
    return false;
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

bool CalculatorCore::percent() {
  stack_[0] = stack_[0] / 100.0f;
  last_error_ = ErrorCode::kNone;
  return true;
}

bool CalculatorCore::reciprocal() {
  if (fabs(stack_[0]) < 0.000001f) {
    setError(ErrorCode::kDivideByZero);
    return false;
  }
  stack_[0] = 1.0f / stack_[0];
  return requireFinite(stack_[0]);
}

bool CalculatorCore::square() {
  stack_[0] = stack_[0] * stack_[0];
  return requireFinite(stack_[0]);
}

bool CalculatorCore::squareRoot() {
  if (stack_[0] < 0.0f) {
    setError(ErrorCode::kOverflow);
    return false;
  }
  stack_[0] = sqrt(stack_[0]);
  return requireFinite(stack_[0]);
}

bool CalculatorCore::naturalLog() {
  if (stack_[0] <= 0.0f) {
    setError(ErrorCode::kOverflow);
    return false;
  }
  stack_[0] = log(stack_[0]);
  return requireFinite(stack_[0]);
}

bool CalculatorCore::commonLog() {
  if (stack_[0] <= 0.0f) {
    setError(ErrorCode::kOverflow);
    return false;
  }
  stack_[0] = log10(stack_[0]);
  return requireFinite(stack_[0]);
}

bool CalculatorCore::exp10() {
  stack_[0] = pow(10.0f, stack_[0]);
  return requireFinite(stack_[0]);
}

bool CalculatorCore::expE() {
  stack_[0] = exp(stack_[0]);
  return requireFinite(stack_[0]);
}

bool CalculatorCore::power() {
  const float exponent = stack_[0];
  const float base = stack_[1];
  const float result = pow(base, exponent);
  if (!requireFinite(result)) {
    return false;
  }

  stack_[1] = result;
  shiftDownFrom(0);
  stack_[kStackDepth - 1] = 0.0f;
  last_error_ = ErrorCode::kNone;
  return true;
}

bool CalculatorCore::trigSin(AngleMode angle_mode) {
  stack_[0] = sin(radiansFromMode(stack_[0], angle_mode));
  return requireFinite(stack_[0]);
}

bool CalculatorCore::trigCos(AngleMode angle_mode) {
  stack_[0] = cos(radiansFromMode(stack_[0], angle_mode));
  return requireFinite(stack_[0]);
}

bool CalculatorCore::trigTan(AngleMode angle_mode) {
  stack_[0] = tan(radiansFromMode(stack_[0], angle_mode));
  return requireFinite(stack_[0]);
}

bool CalculatorCore::pushPi() {
  return push(kPiValue);
}

bool CalculatorCore::pushE() {
  return push(exp(1.0f));
}

void CalculatorCore::store() {
  storage_register_ = stack_[0];
  last_error_ = ErrorCode::kNone;
}

bool CalculatorCore::recall() {
  return push(storage_register_);
}

float CalculatorCore::memory() const {
  return memory_register_;
}

void CalculatorCore::memoryClear() {
  memory_register_ = 0.0f;
  last_error_ = ErrorCode::kNone;
}

void CalculatorCore::memoryAdd() {
  memory_register_ += stack_[0];
  last_error_ = ErrorCode::kNone;
}

void CalculatorCore::memorySubtract() {
  memory_register_ -= stack_[0];
  last_error_ = ErrorCode::kNone;
}

float CalculatorCore::top() const {
  return stack_[0];
}

float CalculatorCore::valueAt(uint8_t index) const {
  if (index >= kStackDepth) {
    return 0.0f;
  }
  return stack_[index];
}

void CalculatorCore::formatTop(char* buffer, size_t length) const {
  formatValue(stack_[0], NumberBase::kDec, buffer, length);
}

void CalculatorCore::formatValue(float value, NumberBase base, char* buffer, size_t length) const {
  if (buffer == nullptr || length == 0) {
    return;
  }

  if (base == NumberBase::kBin || base == NumberBase::kHex) {
    const long integer_value = static_cast<long>(value);
    ultoa(static_cast<unsigned long>(integer_value), buffer, base == NumberBase::kBin ? 2 : 16);
    if (base == NumberBase::kHex) {
      for (size_t i = 0; buffer[i] != '\0'; ++i) {
        buffer[i] = static_cast<char>(toupper(buffer[i]));
      }
    }
    return;
  }

  dtostrf(value, 0, 6, buffer);

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

bool CalculatorCore::requireFinite(float value) {
  if (!isfinite(value)) {
    setError(ErrorCode::kOverflow);
    return false;
  }
  last_error_ = ErrorCode::kNone;
  return true;
}

float CalculatorCore::radiansFromMode(float value, AngleMode angle_mode) const {
  if (angle_mode == AngleMode::kDeg) {
    return value * kPiValue / 180.0f;
  }
  return value;
}

}  // namespace calculator
