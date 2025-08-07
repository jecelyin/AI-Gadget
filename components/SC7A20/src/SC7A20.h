#ifndef __SC7A20_H__
#define __SC7A20_H__
#include <stdint.h>
#include <stdbool.h>


#define SC7A20_I2C_ADDR       0x18


#define WHO_AM_I_REG         0x0F  // 芯片 ID 寄存器

#define CTRL_REG1            0x20  // 控制寄存器 1
#define CTRL_REG2            0x21  // 控制寄存器 2
#define CTRL_REG3            0x22  // 控制寄存器 3
#define CTRL_REG4            0x23  // 控制寄存器 4
#define CTRL_REG5            0x24  // 控制寄存器 5
#define CTRL_REG6            0x25  // 控制寄存器 6
#define INT1_CFG             0x30  // 中断 1 配置寄存器
#define INT1_THS             0x32  // 中断 1 阈值寄存器
#define INT1_DURATION        0x33  // 中断 1 持续时间寄存器
#define CLICK_CFG            0x38  // 点击配置寄存器
#define CLICK_THS            0x3A  // 点击阈值寄存器
#define TIME_LIMIT           0x3B  // 点击时间限制寄存器
#define TIME_LATENCY         0x3C  // 点击时间间隔寄存器
#define TIME_WINDOW          0x3D  // 点击窗口时间寄存器
#define CHIP_ID              0x11  // 芯片 ID 默认值

// 输出数据速率配置值 (ODR - Output Data Rate)
// SC7A20 支持以下几种 ODR 配置：

// ODR（Hz）	ODR 周期（秒）	适用场景
// 1Hz	    1.000 秒	低频检测，如静态倾斜或缓慢移动
// 10Hz	    0.100 秒	较慢的动作检测，如手持设备的平稳移动
// 25Hz 	0.040 秒	日常运动检测，如跑步或走路
// 50Hz 	0.020 秒	普通震动检测或轻微振动检测
// 100Hz	0.010 秒	快速动作检测，如设备掉落或撞击
// 200Hz	0.005 秒	高速检测或工业环境中的高频振动
// 400Hz	0.0025 秒	更高频率的检测，用于机器振动或快速运动检测
// 1250Hz	0.0008 秒	仅适用于某些特殊场景（例如极高速振动检测）
// 5000Hz	0.0002 秒	低功耗模式下的超高速检测（非常高功耗）
#define ODR_POWER_DOWN       0x00  // 电源关断模式
#define ODR_1HZ              0x10  // 正常 / 低功耗模式 (1 Hz)
#define ODR_10HZ             0x20  // 正常 / 低功耗模式 (10 Hz)
#define ODR_25HZ             0x30  // 正常 / 低功耗模式 (25 Hz)
#define ODR_50HZ             0x40  // 正常 / 低功耗模式 (50 Hz)
#define ODR_100HZ            0x50  // 正常 / 低功耗模式 (100 Hz)
#define ODR_200HZ            0x60  // 正常 / 低功耗模式 (200 Hz)
#define ODR_400HZ            0x70  // 正常 / 低功耗模式 (400 Hz)
#define ODR_1250HZ           0x90  // 正常模式 (1250 Hz)
#define ODR_1620HZ_LP        0x80  // 低功耗模式 (1620 Hz)
#define ODR_5000HZ_LP        0x90  // 低功耗模式 (5000 Hz)

// 操作模式配置值
#define MODE_NORMAL          0x00  // 正常模式
#define MODE_LOW_POWER       0x08  // 低功耗模式

// 单击检测配置值
#define CLICK_DETECT_ENABLED 0x31  // 启用单击检测 (CTRL_REG2)
#define CLICK_DETECT_DISABLED 0x00 // 禁用单击检测

// 中断配置值
#define INT1_AOI1_ON_INT1    0x40  // 将 AOI1 映射到 INT1 引脚
#define INT1_AOI2_ON_INT2    0x20  // 将 AOI2 映射到 INT2 引脚
#define INT1_LATCHED         0x02  // 锁存中断
#define INT1_NON_LATCHED     0x00  // 非锁存中断

// 量程范围配置值 (Full-Scale Selection)
#define RANGE_2G             0x00  // ±2g
#define RANGE_4G             0x10  // ±4g
#define RANGE_8G             0x20  // ±8g
#define RANGE_16G            0x30  // ±16g


// 枚举定义模式
typedef enum {
    SC7A20_MODE_VIBRATION,   // 震动模式
    SC7A20_MODE_SINGLE_CLICK // 单击模式
    // 可扩展更多模式
} SC7A20_Mode;

// 函数声明
void SC7A20_Init(SC7A20_Mode mode);
bool SC7A20_NewDataReady();
void SC7A20_ReadAccelerationRaw(int16_t *acc_x, int16_t *acc_y, int16_t *acc_z);
void SC7A20_ReadAcceleration(float *acc_x, float *acc_y, float *acc_z);
// 设置更低的阈值，提高灵敏度（减小阈值）
void SC7A20_SetInterruptThreshold(float threshold_mg, uint8_t range);
/**
 * @brief 配置 CTRL_REG1 寄存器
 * @param odr          输出数据速率 (ODR)，使用 ODR 宏定义值，例如 ODR_50HZ。
 * @param low_power    是否启用低功耗模式，1 = 低功耗，0 = 正常模式。
 * @param enable_axes  启用的轴，按位操作 (X = 0x01, Y = 0x02, Z = 0x04)，例如启用 X 和 Y：0x03。
 */
void SC7A20_SetCTRL_REG1(uint8_t odr, uint8_t low_power, uint8_t enable_axes);
#endif /* __SC7A20_H__ */
