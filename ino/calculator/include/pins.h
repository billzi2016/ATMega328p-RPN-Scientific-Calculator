/*
 * 文件名称：pins.h
 * 文件作用：集中定义全部硬件引脚。
 * 设计意图：
 * 1. 保证 IO 规划与文档一致，避免同一引脚被多个模块重复占用。
 * 2. 明确避开 D0/D1(TX/RX)，保留下载与调试通道。
 */

#ifndef CALCULATOR_PINS_H
#define CALCULATOR_PINS_H

#include <Arduino.h>

namespace calculator {

// LCD2004 标准 4-bit 并口接法。
constexpr uint8_t kLcdPinRs = A0;
constexpr uint8_t kLcdPinE = A1;
constexpr uint8_t kLcdPinD4 = A2;
constexpr uint8_t kLcdPinD5 = A3;
constexpr uint8_t kLcdPinD6 = A4;
constexpr uint8_t kLcdPinD7 = A5;

// 74HC595：键盘行输出。
constexpr uint8_t kShift595Ser = 2;
constexpr uint8_t kShift595Clock = 3;
constexpr uint8_t kShift595Latch = 4;

// 74HC165：键盘列输入。
constexpr uint8_t kShift165Load = 5;
constexpr uint8_t kShift165Clock = 6;
constexpr uint8_t kShift165Data = 7;

// 预留功能口。
constexpr uint8_t kReservedBuzzerOrLed = 8;
constexpr uint8_t kReservedCardDetect = 9;

// TF 卡 SPI：按 Mega 2560 硬件 SPI 引脚定义。
constexpr uint8_t kSdCsPin = 53;
constexpr uint8_t kSpiMosiPin = 51;
constexpr uint8_t kSpiMisoPin = 50;
constexpr uint8_t kSpiClockPin = 52;

}  // namespace calculator

#endif  // CALCULATOR_PINS_H
