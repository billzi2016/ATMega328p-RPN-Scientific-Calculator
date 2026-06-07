# ATMega328P RPN Scientific Calculator

基于 `ATmega328P` 的 `RPN` 科学计算器项目，目标是使用 Arduino 生态实现一套可落地、可维护、具备工业化组织方式的嵌入式计算器方案。

## 项目目标

- 使用 `ATmega328P` 作为主控
- 使用 `LCD2004` 作为显示模块
- 使用 `8x8` 键盘矩阵作为输入设备
- 使用 `74HC595 + 74HC165` 完成矩阵键盘扫描
- 使用 `TF / MicroSD` 模块通过 `SPI` 保存历史算式和结果
- 采用 `RPN` 交互逻辑，并尽量利用全部 `64` 个键位

## 硬件组成

- 主控：`ATmega328P`
- 显示：`LCD2004`
- 键盘：`8x8` 矩阵键盘
- 行驱动：`74HC595`
- 列采样：`74HC165`
- 存储：`TF / MicroSD` `SPI` 模块

## 文档入口

- 产品需求文档：[docs/PRD.md](./docs/PRD.md)
- 接线说明：[docs/IO.md](./docs/IO.md)
- 键盘逐键说明：[docs/KEYBOARD.md](./docs/KEYBOARD.md)
- 项目文件树建议：[docs/PROJECT_TREE.md](./docs/PROJECT_TREE.md)

## 当前阶段

当前仓库以方案设计和文档定义为主，先完成：

- 硬件连接方案
- 键位定义
- 历史文件存储规则
- Arduino 工程目录规范

后续再进入：

- 原型搭建
- 固件代码实现
- 调试与验证
