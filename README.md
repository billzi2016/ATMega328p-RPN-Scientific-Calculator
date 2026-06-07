# Arduino Mega 2560 RPN Scientific Calculator

基于 `Arduino Mega 2560` 的 `RPN` 科学计算器项目，目标是使用 Arduino 生态实现一套可落地、可维护、具备工业化组织方式的嵌入式计算器方案。

## 项目目标

- 使用 `Arduino Mega 2560` 作为主控
- 使用 `LCD2004` 作为显示模块
- 使用 `8x8` 键盘矩阵作为输入设备
- 使用 `74HC595 + 74HC165` 完成矩阵键盘扫描
- 使用 `TF / MicroSD` 模块通过 `SPI` 保存历史算式和结果
- 采用 `RPN` 交互逻辑，并尽量利用全部 `64` 个键位

## 硬件组成

- 主控：`Arduino Mega 2560`
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

当前仓库已经完成文档定义与第一版固件实现：

- 硬件连接方案
- 键位定义
- 历史文件存储规则
- Arduino 工程目录规范
- `RPN` 四级栈与基础/部分科学运算
- `74HC595 + 74HC165` 键盘扫描
- `LCD2004` 主界面/菜单/历史/文件状态界面
- `TF / MicroSD` 会话日志写入与最近历史读取
- `Mega 2560` 目标板适配
- `calculator_core` 自检 sketch

当前验证方式：

- 主工程编译检查
- `calculator_core` 自检工程编译检查

仍需继续完成的工作：

- 更完整的文件浏览与跨会话历史浏览
- 更多副层键位功能
- 实板联调与长期稳定性验证
