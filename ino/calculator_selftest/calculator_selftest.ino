/*
 * 文件名称：calculator_selftest.ino
 * 文件作用：提供一个可单独编译和上板运行的 CalculatorCore 自检 sketch。
 * 设计意图：
 * 1. 让核心计算逻辑具备最小可执行测试入口，而不是只依赖主工程编译。
 * 2. 上板后可通过串口直接看到 PASS/FAIL 结果，便于快速验证核心功能。
 */

#include "include/calculator_core.h"

static calculator::CalculatorCore g_core;
static bool g_all_passed = true;

/*
 * 函数作用：记录单项测试结果。
 */
void reportCheck(const __FlashStringHelper* label, bool passed) {
  Serial.print(label);
  Serial.print(F(": "));
  Serial.println(passed ? F("PASS") : F("FAIL"));
  if (!passed) {
    g_all_passed = false;
  }
}

/*
 * 函数作用：比较浮点结果是否在容差范围内。
 */
bool nearlyEqual(float actual, float expected, float tolerance) {
  return fabs(actual - expected) <= tolerance;
}

/*
 * 函数作用：执行 CalculatorCore 的基础功能自检。
 */
void runCoreSelfTest() {
  g_core.begin();

  g_core.push(2.0f);
  g_core.push(3.0f);
  reportCheck(F("ADD"), g_core.applyBinary(calculator::BinaryOp::kAdd) && nearlyEqual(g_core.top(), 5.0f, 0.0001f));

  g_core.push(4.0f);
  reportCheck(F("MUL"), g_core.applyBinary(calculator::BinaryOp::kMultiply) && nearlyEqual(g_core.top(), 20.0f, 0.0001f));

  g_core.push(9.0f);
  reportCheck(F("SQRT"), g_core.squareRoot() && nearlyEqual(g_core.top(), 3.0f, 0.0001f));

  g_core.pushPi();
  reportCheck(F("PI"), nearlyEqual(g_core.top(), 3.14159f, 0.01f));

  g_core.push(30.0f);
  reportCheck(F("SIN_DEG"), g_core.trigSin(calculator::AngleMode::kDeg) && nearlyEqual(g_core.top(), 0.5f, 0.01f));

  g_core.push(2.0f);
  g_core.store();
  g_core.push(10.0f);
  reportCheck(F("STO_RCL"), g_core.recall() && nearlyEqual(g_core.top(), 2.0f, 0.0001f));

  g_core.push(8.0f);
  g_core.push(2.0f);
  reportCheck(F("DIV"), g_core.applyBinary(calculator::BinaryOp::kDivide) && nearlyEqual(g_core.top(), 4.0f, 0.0001f));
}

/*
 * 函数作用：Arduino 初始化入口。
 */
void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }

  Serial.println(F("CalculatorCore Self Test"));
  runCoreSelfTest();
  Serial.println(g_all_passed ? F("ALL PASS") : F("HAS FAIL"));
}

/*
 * 函数作用：自检 sketch 不需要循环逻辑。
 */
void loop() {
}
