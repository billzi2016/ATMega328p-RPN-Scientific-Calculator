# 项目文件树建议

## 1. 说明
本文件用于描述推荐的项目组织方式。  
当前阶段它是“规范文档”，不是要求现在就把所有目录和源码都建出来。

## 2. 推荐文件树

```text
ATMega328p-RPN-Scientific-Calculator/
├── README.md
├── docs/
│   ├── PRD.md
│   ├── IO.md
│   ├── KEYBOARD.md
│   └── PROJECT_TREE.md
├── ino/
│   └── calculator/
│       ├── calculator.ino
│       ├── src/
│       │   ├── app.cpp
│       │   ├── calculator_core.cpp
│       │   ├── display_lcd2004.cpp
│       │   ├── keyboard_matrix.cpp
│       │   ├── storage_sd.cpp
│       │   ├── history_manager.cpp
│       │   └── diagnostics.cpp
│       ├── include/
│       │   ├── app.h
│       │   ├── calculator_core.h
│       │   ├── display_lcd2004.h
│       │   ├── keyboard_matrix.h
│       │   ├── storage_sd.h
│       │   ├── history_manager.h
│       │   ├── diagnostics.h
│       │   ├── pins.h
│       │   ├── config.h
│       │   └── types.h
│       ├── lib/
│       └── testdata/
│           ├── sample_log_1.txt
│           └── sample_log_2.txt
```

## 3. 目录职责

### 3.1 `README.md`
- 仓库根说明文件，面向 GitHub 首页展示项目简介、特性、硬件组成与文档入口

### 3.2 `docs/`
- 存放需求、接线、文件树、协议、测试说明等文档

### 3.3 `ino/calculator/`
- Arduino 主工程目录
- 其中 `.ino` 只保留入口
- 复杂逻辑全部拆到 `src/` 与 `include/`

### 3.4 `src/`
- 放各模块实现文件 `.cpp`
- 按功能拆分，不要把所有逻辑堆在单文件

### 3.5 `include/`
- 放模块头文件 `.h`
- 统一声明接口、类型、常量和引脚定义

### 3.6 `lib/`
- 放需要单独管理的第三方库或本地库封装

### 3.7 `testdata/`
- 放日志格式样例、测试输入、导入导出示例

## 4. Arduino 代码组织规则
- `calculator.ino` 仅保留：
  - `setup()`
  - `loop()`
- `pins.h`：
  - 集中定义全部引脚
- `config.h`：
  - 集中定义栈深、缓冲区、路径、显示参数
- `history_manager.*`：
  - 专门负责 `1.txt / 2.txt / 3.txt` 顺延策略
- 全部模块使用中文注释，重点解释设计意图和边界条件
- 每个 `.ino / .cpp / .h` 文件开头必须有文件头注释，说明职责、设计目的、依赖关系
- 每个函数必须有中文函数注释，说明参数、返回值、调用场景、异常或边界处理
- 关键流程必须有中文行注释或段注释，尤其是键盘扫描、LCD 刷新、SD 写入、错误处理
- 重要常量、引脚、状态量、定时参数必须带中文说明，不能只保留裸数字

## 5. 文件命名建议
- 文档：大写语义名，如 `PRD.md`
- 头文件：小写下划线，如 `calculator_core.h`
- 实现文件：与头文件同名，如 `calculator_core.cpp`
- Arduino 主文件：与目录同名，如 `calculator.ino`

## 6. 不建议的组织方式
- 不建议把全部逻辑写进一个 `.ino`
- 不建议把引脚号散落在多个文件
- 不建议把 SD 卡、键盘、显示、计算核心强耦合
- 不建议无分层直接在界面代码里写文件系统逻辑
