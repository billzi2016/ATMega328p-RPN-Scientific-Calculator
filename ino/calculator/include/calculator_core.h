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
   * 函数作用：执行四级栈上卷。
   */
  bool rollUp();

  /*
   * 函数作用：执行四级栈下卷。
   */
  bool rollDown();

  /*
   * 函数作用：执行二元运算。
   */
  bool applyBinary(BinaryOp op);

  /*
   * 函数作用：把栈顶值乘以 -1。
   */
  bool toggleSign();

  /*
   * 函数作用：把栈顶按百分数处理。
   */
  bool percent();

  /*
   * 函数作用：对栈顶执行倒数。
   */
  bool reciprocal();

  /*
   * 函数作用：对栈顶执行平方。
   */
  bool square();

  /*
   * 函数作用：对栈顶执行平方根。
   */
  bool squareRoot();

  /*
   * 函数作用：对栈顶执行自然对数。
   */
  bool naturalLog();

  /*
   * 函数作用：对栈顶执行常用对数。
   */
  bool commonLog();

  /*
   * 函数作用：对栈顶执行 10 的幂。
   */
  bool exp10();

  /*
   * 函数作用：对栈顶执行 e 的幂。
   */
  bool expE();

  /*
   * 函数作用：对 Y 和 X 执行幂运算 Y^X。
   */
  bool power();

  /*
   * 函数作用：按当前角度模式计算三角函数。
   */
  bool trigSin(AngleMode angle_mode);
  bool trigCos(AngleMode angle_mode);
  bool trigTan(AngleMode angle_mode);

  /*
   * 函数作用：按当前角度模式计算反三角函数。
   */
  bool arcSin(AngleMode angle_mode);
  bool arcCos(AngleMode angle_mode);
  bool arcTan(AngleMode angle_mode);

  /*
   * 函数作用：执行扩展一元科学函数。
   */
  bool absoluteValue();
  bool cubeRoot();
  bool decimalDegreesToDms();
  bool dmsToDecimalDegrees();

  /*
   * 函数作用：把数学常数压入栈顶。
   */
  bool pushPi();
  bool pushE();

  /*
   * 函数作用：把栈顶值存入 STO 寄存器。
   */
  void store();

  /*
   * 函数作用：把栈顶值存入指定编号的寄存器槽。
   */
  void storeSlot(uint8_t slot_index);

  /*
   * 函数作用：仅清空四级栈内容，不影响寄存器。
   */
  void clearStack();

  /*
   * 函数作用：从 STO 寄存器回读并压栈。
   */
  bool recall();

  /*
   * 函数作用：从指定编号寄存器槽回读并压栈。
   */
  bool recallSlot(uint8_t slot_index);

  /*
   * 函数作用：读取 M 寄存器当前值。
   */
  float memory() const;

  /*
   * 函数作用：清空 M 寄存器。
   */
  void memoryClear();

  /*
   * 函数作用：把栈顶累加到 M 寄存器。
   */
  void memoryAdd();

  /*
   * 函数作用：把栈顶从 M 寄存器中减去。
   */
  void memorySubtract();

  /*
   * 函数作用：获取当前栈顶值。
   */
  float top() const;

  /*
   * 函数作用：读取指定层级的栈值。
   */
  float valueAt(uint8_t index) const;

  /*
   * 函数作用：把栈顶值格式化为可显示字符串。
   * 参数：
   * - buffer：输出缓冲区
   * - length：缓冲区长度
   */
  void formatTop(char* buffer, size_t length) const;

  /*
   * 函数作用：按当前显示进制把任意数值格式化为字符串。
   */
  void formatValue(float value, NumberBase base, char* buffer, size_t length) const;

  /*
   * 函数作用：返回最近一次错误码。
   */
  ErrorCode last_error() const;

 private:
  float stack_[kStackDepth];
  float storage_registers_[10] = {};
  float memory_register_ = 0.0f;
  ErrorCode last_error_ = ErrorCode::kNone;

  void setError(ErrorCode code);
  void shiftDownFrom(uint8_t index);
  bool requireFinite(float value);
  float radiansFromMode(float value, AngleMode angle_mode) const;
  float angleFromRadians(float value, AngleMode angle_mode) const;
};

}  // namespace calculator

#endif  // CALCULATOR_CORE_H
