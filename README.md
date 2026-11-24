# STM32F407 六轴机械臂动作模块

用于 STM32F407ZGT6。提供两个阻塞式动作接口：`pick_above`（抓取、搬运、松爪）和 `reset`（从记录的目标姿态用 5 秒回零）。

## 接线

数组角度依次对应 P1～P6。

| 关节 | 功能 | STM32 信号引脚 |
|---|---|---|
| P1 | 底座旋转 | PA1 |
| P2 | 肩部俯仰 | PC3 |
| P3 | 肘部俯仰 | PA3 |
| P4 | 腕部俯仰 | PA5 |
| P5 | 夹爪旋转 | PC4 |
| P6 | 夹爪开合 | PB0 |

舵机使用符合其规格的外部电源，电源 GND 与 STM32 GND 共地；信号线接上述引脚。不要用 STM32 的 3.3V 给舵机供电。

## 怎么调用

先执行 `HAL_Init()` 和本工程的 `SystemClock_Config()`，再调用：

```c
#include "robotarm_actions.h"

if (RobotArm_Init() != SERVO_OK) Error_Handler();       // 首次直接输出全零
if (RobotArm_PickAbove() != SERVO_OK) Error_Handler();  // 执行 pick_above
if (RobotArm_Reset() != SERVO_OK) Error_Handler();      // 用5秒回全零
```

- `RobotArm_Init()` 只初始化并命令全零；重复调用不会改变已有姿态。内部已配置 GPIO/TIM2，不必另调 `MX_TIM2_Init()`。
- `RobotArm_PickAbove()` 要求记录的起点是全零，否则返回 `SERVO_START_POSE_MISMATCH`；需要再次抓取时先调用 `RobotArm_Reset()`。
- `RobotArm_Reset()` 从最近完成的指令姿态插值回零，不写死抓取结束位置。
- `RobotArm_GetCommandedPose(out)` 读取记录的六个角度。
- 返回值为 `ServoStatus`；错误定义见 [servo.h](Core/Inc/servo.h)。接口须从同一主循环/任务串行调用，不在中断中调用。

[main.c](Core/Src/main.c) 是完整示例：初始化后显式抓取一次，随后保持；默认不自动复位、不循环。仅调用初始化不会触发抓取。

## pick_above 动作

上电初始化直接命令 `[0,0,0,0,0,0]`，之后按下表同步渐变：

| 顺序 | 目标角度 [P1,P2,P3,P4,P5,P6] | 运动时间 | 随后保持 |
|---|---|---|---|
| 1 | [0,135,10,35,0,0] | 5秒 | 1秒 |
| 2 | [0,135,10,35,0,90] | 2秒 | — |
| 3 | [0,90,90,35,0,90] | 3秒 | — |
| 4 | [180,90,90,35,0,90] | 2秒 | — |
| 5 | [180,60,0,0,0,90] | 3秒 | — |
| 6 | [180,60,0,0,0,0] | 1秒 | 持续保持 |

搬运中夹爪保持 90°，最后打开到 0°。函数约耗时 17.24 秒，包含各段末 40ms 的 PWM 帧生效余量。`reset` 约耗时 5.04 秒。

## 怎么修改

| 需求 | 修改位置 |
|---|---|
| 修改抓取姿态、运动时间、保持时间 | [servo_motion.c](Core/Src/servo_motion.c) 的 `pick_above_stages`；时间单位为毫秒 |
| 增减动作段数 | 同时修改 [servo_motion.h](Core/Inc/servo_motion.h) 的 `PICK_ABOVE_STAGE_COUNT` |
| 修改复位时间 | [robotarm_actions.c](Core/Src/robotarm_actions.c) 的 `RobotArm_Reset()`，当前为 `5000U` |
| 新增命名动作 | 在 `robotarm_actions.h/.c` 增加接口，复用 `Servo_MoveTo()` |
| 修改接线 | [tim.c](Core/Src/tim.c)：同步修改 `ports[]/pins[]`、GPIO 时钟/初始化，以及帧开始时拉高 GPIO 的掩码；并同步 `robotarm.ioc` |

例如，单段数据 `{{0,135,10,35,0,0},5000U,1000U}` 表示用 5 秒到目标，再保持 1 秒。

也可以直接移动到自定义目标：

```c
const int16_t target[6] = {30,60,90,45,20,10};
if (Servo_MoveTo(target, 2000U) != SERVO_OK) Error_Handler();
```

角度范围 0～180°；运动时间必须是 20ms 的整数倍，范围 20～600000ms。非法输入会返回错误，不会自动截断或环绕。

## 构建与下载

需要 Arm GNU Toolchain（`arm-none-eabi-gcc` 等）、CMake ≥ 3.22、Ninja，并将这些工具加入 PATH。

```sh
cmake --preset Debug
cmake --build --preset Debug
```

输出位于 `build/Debug/robotarm.elf`、`robotarm.hex`、`robotarm.bin`。可使用 STM32CubeProgrammer 下载 HEX 并校验。

本板 USB_UART Type-C 为 CH340 串口；串口下载前按住 KEY0，短按并松开 RST，再松开 KEY0。将下例 COM5 换成实际端口：

```sh
STM32_Programmer_CLI -c port=COM5 br=38400 -w build/Debug/robotarm.hex -v
STM32_Programmer_CLI -c port=COM5 br=38400 -g 0x08000000
```

启动后会执行 main.c 中的动作。也可使用 ST-LINK/SWD 下载。

## 验证

主机测试直接链接生产动作代码，用模拟时间/PWM 检查动作时序、180°底座目标、不同结束姿态的复位、重复调用及错误处理；PWM 测试另覆盖 10000 个模拟周期。使用本机原生 GCC（不是 Arm 交叉编译器）：

```sh
cmake -S tests -B build/host-tests -G Ninja -DCMAKE_C_COMPILER=gcc
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```

## 硬件与位置约定

- 时钟：HSI 16MHz，AHB/APB1/APB2 不分频。TIM2 被本模块独占；不要直接套用不同的系统时钟。
- PWM：50Hz，0°/90°/180°分别对应 500/1500/2500μs。PC3/PC4 没有硬件 PWM 复用，六路通过 TIM2 中断驱动 GPIO。
- 记录的是指令位置，不是传感器反馈。上电第一次定位速度不受控；复位依赖舵机正常跟随、持续供电且未被外力改变。断电、堵转或手动扳动后，记录不代表实际位置。
- 本项目使用关节角线性插值，没有碰撞检测。机械零点、方向和安全角度需按实际安装校准。
- `robotarm.ioc` 供 CubeMX 查看配置；重新生成时需保留自定义 `tim.c` 和 `TIM2_IRQHandler`，检查 CMake 集成。

HAL/CMSIS 依赖已附带，保留各自许可证：[HAL](Drivers/STM32F4xx_HAL_Driver/LICENSE.md)、[CMSIS Core](Drivers/CMSIS/LICENSE.txt)、[CMSIS Device F4](Drivers/CMSIS/Device/ST/STM32F4xx/LICENSE.md)。
