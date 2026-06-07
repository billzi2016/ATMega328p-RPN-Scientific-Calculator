/*
 * 文件名称：calculator.ino
 * 文件作用：Arduino 工程主入口，仅负责调用应用层初始化与周期更新。
 * 设计意图：
 * 1. 把 setup()/loop() 之外的业务逻辑全部下沉到 App 模块。
 * 2. 避免把键盘、显示、存储、计算核心直接堆在 .ino 中，降低后续维护成本。
 */

#include "include/app.h"

static calculator::App g_app;

/*
 * 函数名称：setup
 * 函数作用：完成系统一次性初始化。
 * 调用时机：Arduino 上电或复位后自动调用一次。
 */
void setup() {
  g_app.begin();
}

/*
 * 函数名称：loop
 * 函数作用：驱动主状态机循环运行。
 * 调用时机：Arduino 初始化完成后持续循环调用。
 */
void loop() {
  g_app.update();
}
