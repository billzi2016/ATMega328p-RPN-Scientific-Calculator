/*
 * 文件名称：calculator_core.h
 * 文件作用：声明 RPN 计算核心接口。
 * 设计意图：
 * 1. 让栈管理、基本运算和错误处理从界面层解耦。
 * 2. 为后续增加科学函数、程序员模式和更深栈结构保留扩展点。
 */

#ifndef CALCULATOR_CORE_H
#define CALCULATOR_CORE_H

#include <Arduino.h>

#include "config.h"
#include "types.h"

namespace calculator {

class CalculatorCore {
 public:
  /*
   * 函数作用：初始化或重置栈状态。
   */
  void begin();
  void reset();

  /*
   * 函数作用：把一个数值压入栈顶。
   * 返回值：
   * - true：压栈成功
   * - false：压栈失败，通常表示栈溢出
   */
  bool push(float value);

  /*
   * 函数作用：复制栈顶，符合 RPN 中 ENTER 的核心语义。
   */
  bool enter();

  /*
   * 函数作用：弹出栈顶一个值。
   */
  bool drop();

  /*
   * 函数作用：交换 X 与 Y。
   */
  bool swap();

  /*
   * 函数作用：复制栈顶值。
   */
  bool dup();

  /*
   * 函数作用：执行二元运算。
   */
  bool applyBinary(BinaryOp op);

  /*
   * 函数作用：把栈顶值乘以 -1。
   */
  bool toggleSign();

  /*
   * 函数作用：获取当前栈顶值。
   */
  float top() const;

  /*
   * 函数作用：把栈顶值格式化为可显示字符串。
   * 参数：
   * - buffer：输出缓冲区
   * - length：缓冲区长度
   */
  void formatTop(char* buffer, size_t length) const;

  /*
   * 函数作用：返回最近一次错误码。
   */
  ErrorCode last_error() const;

 private:
  float stack_[kStackDepth];
  ErrorCode last_error_ = ErrorCode::kNone;

  void setError(ErrorCode code);
  void shiftDownFrom(uint8_t index);
};

}  // namespace calculator

#endif  // CALCULATOR_CORE_H
