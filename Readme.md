# SUPERCAP

基于 STM32G474RBTx 的超级电容管理与闭环控制项目。代码按功能分层，便于替换算法或移植外设。

---

## 文件树

```
SUPERCAP/
├── SUPERCAP.ioc                      # CubeMX 项目文件（外设/时钟/引脚配置入口）
├── CMakeLists.txt                     # 顶层 CMake 构建脚本
├── CMakePresets.json                  # CMake preset（Debug / Release + Ninja）
├── STM32G474XX_FLASH.ld              # 链接脚本（Flash 128K + RAM 128K）
├── startup_stm32g474xx.s             # 启动汇编
├── Readme.md
├── .mxproject                         # CubeMX 元数据
├── .clangd                            # clangd 配置（LSP 跳转准确度）
│
├── cmake/
│   ├── gcc-arm-none-eabi.cmake        # ARM GCC 交叉编译工具链定义
│   ├── starm-clang.cmake              # 备用：clang 工具链
│   └── stm32cubemx/
│       └── CMakeLists.txt             # STM32Cube HAL / CMSIS 库构建入口
│
├── .vscode/
│   ├── c_cpp_properties.json          # IntelliSense 配置（includePath, defines）
│   ├── launch.json                    # 调试启动（J-Link GDB Server）
│   └── settings.json                  # VS Code 编辑器 & CMake Tools 偏好
│
├── Core/                              # ---- CubeMX 生成 ----
│   ├── Inc/
│   │   ├── main.h                     # MCU 引脚宏、外设句柄声明
│   │   ├── adc.h / dma.h / fdcan.h
│   │   ├── gpio.h / hrtim.h / tim.h
│   │   ├── stm32g4xx_hal_conf.h       # HAL 驱动模块开关
│   │   └── stm32g4xx_it.h             # 中断服务函数声明
│   └── Src/
│       ├── main.c                     # 主函数入口，外设初始化
│       ├── adc.c / dma.c / fdcan.c
│       ├── gpio.c / hrtim.c / tim.c
│       ├── stm32g4xx_hal_msp.c        # HAL 外设 MSP 回调
│       ├── stm32g4xx_it.c             # 中断处理（转发到 HAL_callback）
│       ├── system_stm32g4xx.c         # 系统时钟配置
│       ├── syscalls.c / sysmem.c      # 标准库桩函数
│
├── User/                              # ---- 用户应用代码 ----
│   ├── data/
│   │   ├── module_data.h              # 核心数据结构定义（datacollect, PID_Configs 等）
│   │   ├── module_data.c              # 全局变量实例 & 校准配置数组
│   │   ├── const_data.h               # 所有系统常量声明
│   │   └── const_data.c               # 所有系统常量定义（阈值、限值、校准表）
│   │
│   ├── interface/                     # 模块对外接口头文件
│   │   ├── HAL_callback.h             # HAL 中断回调入口声明
│   │   ├── Data_collect.h             # ADC 数据采集接口
│   │   ├── MOS_driver.h               # MOS 驱动接口
│   │   ├── CAN_communicate.h          # CAN 通信接口
│   │   └── SuperCap_init.h            # 超级电容初始化（软启动）
│   │
│   ├── math_tools/                    # 算法实现
│   │   ├── ADC_Calibration.h / .c     # ADC 转换（counts → 物理值）
│   │   └── PID_controller.h / .c      # PID 控制器（功率环 / 电流环）
│   │
│   ├── v1/src/                        # C 版本模块实现
│   │   ├── Data_collect.c             # HRTIM 触发 ADC → DMA 缓冲 → 分发
│   │   ├── HAL_callback.c             # 所有中断回调逻辑（保护、PID、CAN）
│   │   ├── CAN_communicate.c          # CAN 收发、断联检测、状态上报
│   │   ├── MOS_driver.c               # HRTIM PWM 占空比控制
│   │   └── SuperCap_init.c            # 软启动、自动重启逻辑
│   │
│   ├── v2/src/                        #（预留）C++ 版本实现
│   └── test/                          # 主机端算法测试用例
│
└── Drivers/                           # ---- 平台库（只读） ----
    ├── CMSIS/
    │   ├── Include/                    # CMSIS Core 头文件（core_cm4.h 等）
    │   ├── Device/ST/STM32G4xx/
    │   │   ├── Include/                # STM32G474 器件头文件
    │   │   └── Source/Templates/
    │   │       ├── arm/ / gcc/ / iar/  # 各工具链启动文件模板
    │   │       └── system_stm32g4xx.c
    │   ├── DSP/                        # CMSIS-DSP 库（BasicMath / Filtering / Matrix ...）
    │   └── NN/                         # CMSIS-NN 神经网络推理库
    └── STM32G4xx_HAL_Driver/
        ├── Inc/                        # HAL / LL 驱动头文件（~90 个）
        └── Src/                        # HAL / LL 驱动源文件（~85 个）
```

---

## 快速上手

### 环境配置

**1. ARM GCC 工具链**

下载 [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain) 并添加到系统 `PATH`（当前使用 13.3.1+st.9 版本）。

```powershell
arm-none-eabi-gcc --version
arm-none-eabi-g++ --version
```

工具链路径硬编码于 `cmake/gcc-arm-none-eabi.cmake` 和 `.vscode/c_cpp_properties.json` 中，若安装位置不同需修改：

- `cmake/gcc-arm-none-eabi.cmake:3-5` — `CMAKE_C_COMPILER` / `CXX_COMPILER` / `ASM_COMPILER`
- `.vscode/c_cpp_properties.json:21` — `compilerPath`

**2. 构建工具**

安装 CMake ≥ 3.22 和 Ninja。

```powershell
cmake --version
ninja --version
```

**3. VS Code 插件**

| 插件 | 用途 |
|------|------|
| `CMake Tools` | 构建配置、预设选择 |
| `Cortex-Debug` | J-Link / ST-Link 调试 |
| `C/C++` / `clangd` | IntelliSense / LSP 支持 |

打开项目根目录，CMake Tools 自动检测 `CMakePresets.json`，选择 Debug 或 Release 预设即可配置。

**4. CubeMX（修改外设配置时使用）**

打开 `SUPERCAP.ioc` 修改引脚 / 外设 / 时钟树配置，重新生成代码后覆盖 `Core/` 目录。

### 编译

```powershell
# 使用 CMake preset（推荐）
cmake --preset Debug
cmake --build build/Debug

# 或直接
cd build\Debug
cmake --build . --config Debug
```

构建产物：
- `build/Debug/SUPERCAP.elf` — ELF 可执行文件（含调试信息）
- `build/Debug/SUPERCAP.bin` / `.hex` — 烧录镜像
- `build/Debug/SUPERCAP.map` — 内存映射报告

编译选项（`CMakeLists.txt:25`）：`-O3 -mfloat-abi=hard -mfpu=fpv4-sp-d16`，启用 FPU 硬浮点。

### 烧录

```powershell
# STM32CubeProgrammer（推荐）
STM32_Programmer_CLI -c port=SWD -w build/Debug/SUPERCAP.elf -rst

# OpenOCD + J-Link / ST-Link
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program build/Debug/SUPERCAP.elf verify reset exit"
```

### 调试

1. 连接 J-Link / ST-Link 调试器。
2. VS Code 按 `F5`，选择 `STM32Cube: STM32 Launch JLink GDB Server` 启动调试。
3. 可查看变量（`adc_data`、`power_pid_configs`、`current_pid_configs`）、设置断点、单步执行。

---

## 模块架构

```
┌───────────────────────────────────────────────────────────┐
│  HAL 中断层                                               │
│  HAL_callback.c                                           │
│  ├─ HAL_ADC_ConvCpltCallback                              │
│  │   └─ Data_distribute()    # 分发原始 ADC counts         │
│  ├─ HAL_TIM_PeriodElapsedCallback (TIM8 50kHz)            │
│  │   ├─ ADC_Transformer_*()  # 校准转换                    │
│  │   ├─ 过压保护 (V_CAP_TF > 25V → MosDriver_stop)        │
│  │   ├─ 电流环 PID (I_CAP_TF, 50kHz)                      │
│  │   ├─ 功率环 PID (P_chassis, 20kHz)                     │
│  │   ├─ 低压保护 (V_CAP < 10V & I_CAP < -10A)             │
│  │   └─ 动态最大占空比 (V_CHASSIS 前馈)                    │
│  └─ HAL_TIM_PeriodElapsedCallback (TIM16 1kHz)            │
│      ├─ CAN_send()              # 状态上报                 │
│      ├─ CAN_disconnect_detection()                        │
│      └─ 掉电检测 (电压窗口滤波 + 电流阈值)                  │
└───────────────────────────────────────────────────────────┘
```

### 核心数据流

```
ADC 采样 (HRTIM 触发)
   ↓
DMA → DataArray[4]
   ↓
Data_distribute() → adc_data.{V_CHASSIS_ADC, I_CHASSIS_ADC, I_CAP_ADC, V_CAP_ADC}
   ↓ (uint16_t, 0~4095)
ADC_Transformer_voltage()  → adc_data.{V_CHASSIS_TF, V_CAP_TF}      (float, 伏特)
ADC_Transformer_current()  → adc_data.{I_CHASSIS_TF, I_CAP_TF}      (float, 安培)
   ↓
PID 控制 (功率环 → 电流环)
   ↓
MosDriver_dutylimit() → HRTIM PWM 输出 → 半桥 MOS
```

> **重要：** `_ADC` 后缀字段为原始 uint16_t counts，`_TF` 后缀字段为转换后的 float 物理值。控制逻辑与保护判断须使用 `_TF` 字段。

### 模块说明

| 模块 | 文件 | 功能 |
|------|------|------|
| **Data_collect** | `v1/src/Data_collect.c` | HRTIM Compare 触发 ADC1 扫描，DMA 搬运 4 通道数据，`Data_distribute()` 写入 `adc_data` 的 `_ADC` 字段 |
| **ADC_Calibration** | `math_tools/ADC_Calibration.c` | ADC counts → 物理值转换。电压：`counts * 36.3 / 4096`（11:1 分压 × 3.3V ref / 12bit）。电流：`counts * 0.0079345 - 16.2`（INA240A2 50V/V 增益，0.002Ω 采样电阻，1.65V 偏置） |
| **PID_controller** | `math_tools/PID_controller.c` | 增量式 PID。功率环（目标功率 → 目标电流）→ 电流环（目标电流 → PWM 占空比） |
| **MOS_driver** | `v1/src/MOS_driver.c` | HRTIM 半桥 PWM 生成。`OUT_MIN` / `OUT_MAX` 限制占空比，用于保护和软启动 |
| **CAN_communicate** | `v1/src/CAN_communicate.c` | FDCAN1 收发。接收裁判系统（RMCS）功率目标与使能指令；上报电压 / 电流 / 功率状态。5 秒无接收触发断联保护 |
| **SuperCap_init** | `v1/src/SuperCap_init.c` | 上电软启动：按电容 / 底盘电压比计算初始占空比，逐步放开 `OUT_MIN`；CAN 恢复后自动重新启动 |
| **HAL_callback** | `v1/src/HAL_callback.c` | 所有中断回调集中处理（ADC 完成、TIM8 50kHz、TIM16 1kHz） |

### 关键数据结构

```
datacollect (module_data.h:42-54)
├── DataArray[4]       uint16_t    DMA 原始缓冲区
├── V_CHASSIS_ADC      uint16_t    底盘电压 ADC counts
├── I_CHASSIS_ADC      uint16_t    底盘电流 ADC counts
├── I_CAP_ADC          uint16_t    电容电流 ADC counts
├── V_CAP_ADC          uint16_t    电容电压 ADC counts
├── V_CHASSIS_TF       float       底盘电压 (V)
├── I_CHASSIS_TF       float       底盘电流 (A)
├── I_CAP_TF           float       电容电流 (A)
└── V_CAP_TF           float       电容电压 (V)

PID_Configs (module_data.h:57-69)
├── Kp / Ki / Kd       float       PID 增益
├── error / pre_error  float       当前 / 上次误差
├── pre_pre_error      float       上上次误差（增量式）
├── output             float       控制器输出
├── OUT_MAX / OUT_MIN  float       输出限幅
├── target_value       float       目标值
└── SWITCH             bool        启停控制

mosdriver (module_data.h:18-28)
├── driver_A / driver_B  uint32_t  通道占空比原始值
├── CHANNEL_CYCLE        uint32_t  HRTIM 周期
├── OUT_MAX / OUT_MIN    float     占空比限幅 (0.0 ~ 1.0)
└── ENABLE               bool      输出使能
```

---

## 关键数值

### 硬件参数

| 参数 | 值 | 说明 |
|------|-----|------|
| MCU | STM32G474RBTx | Cortex-M4, 170MHz, FPU |
| Flash / RAM | 128KB / 128KB | |
| ADC 分辨率 | 12bit (0 ~ 4095) | |
| ADC 参考电压 | 3.3V | VREF+ 内部参考 |
| HRTIM 周期 | 27200 counts | 对应 50kHz PWM 频率 |
| 电压分压比 | 11:1 | 底盘 / 电容端均使用 |
| 电流采样电阻 | 0.002Ω | INA240A2 放大器 |
| 电流放大器增益 | 50 V/V | INA240A2，偏置 = VREF/2 |
| CAN 控制器 | FDCAN1 + FDCAN3 | 1Mbps |

### 转换公式

```
V_physical = V_ADC × 36.3 / 4096          （电压：counts → 伏特）
I_physical = I_ADC × 0.0079345 − 16.2     （电流：counts → 安培）
```

### 工作限值

| 参数 | 下限 | 上限 | 说明 |
|------|------|------|------|
| 底盘电压 V_CHASSIS | 20.0V | 24.0V | 电池端 |
| 电容电压 V_CAP | 4.0V | 23.0V | 电容组端（过压保护 25V） |
| 电容电流 I_CAP | -10A | +10A | 充电为正，放电为负 |
| 底盘功率 P_CHASSIS | 35W | 120W | 目标功率范围 |
| 最大占空比 | 1% | 53.5% | 23V / (20V+23V) |
| 电容低压保护 | — | < 10V + I_CAP < -10A | 抬高最低占空比防放电 |
| CAN 断联超时 | — | 5000ms | 接收裁判系统指令超时 |
| 掉电检测超时 | — | 1000ms | 电压 & 电流同时异常 |

### 保护逻辑

| 保护类型 | 触发条件 | 动作 |
|----------|----------|------|
| 过压保护 | `V_CAP_TF > 25.0V` | 立即停止 MOS 驱动，重置 PID |
| 低压保护 | `V_CAP_TF ≤ 10.0V && I_CAP_TF ≤ -10.0A` | 抬高 `OUT_MIN` 阻止继续放电 |
| 掉电检测 | 电流异常（`I_CAP_TF < -0.2 && I_CHASSIS_TF < 0.3`）&& 电压低（`V_CHASSIS < 19V`）持续 1000ms | 停止 MOS 驱动，重置 PID |
| CAN 断联 | 5000ms 未收到裁判系统指令 | 停止 MOS 驱动，等待恢复后自动重启 |
| 软启动 | 上电 / CAN 恢复 | `OUT_MIN` 从电压比分压比逐步放开 |

---

## 开发引导

### 新增模块流程

1. **声明接口**：在 `User/interface/` 下创建 `模块名.h`，声明类型定义、函数原型、extern 变量。
2. **实现模块**：在 `User/v1/src/`（C）或 `User/v2/src/`（C++）下创建 `模块名.c`，包含对应头文件。
3. **注册回调**：在 `HAL_callback.c` 的定时器或 ADC 回调中调用模块接口。
4. **更新构建**：项目 `CMakeLists.txt:62-64` 使用 `GLOB_RECURSE` 自动收集 `User/` 下的 `.c` 文件，新增源文件无需手动修改 CMake。
5. **编写测试**：在 `User/test/` 下编写测试用例，主机环境编译运行验证算法逻辑。
6. **上板测试**：烧录到目标板，通过 CAN 或 J-Link 调试观测变量。

### 修改 CubeMX 外设配置

1. 打开 `SUPERCAP.ioc`，修改外设参数、引脚映射、时钟树。
2. 点击 `GENERATE CODE` 重新生成。
3. 覆盖 `Core/` 目录后，检查 `main.c` 中的初始化是否与 `HAL_callback.c` 兼容。
4. 如有新增外设引脚宏（如 `FDCAN3`），在 `User/` 层代码中引用 `main.h` 中对应的宏。

### 调整保护阈值

所有阈值统一定义在 `User/data/const_data.c`，通过 `const_data.h` 声明。常见调整项：

- 功率目标默认值：`DEFAULT_POWER_CHASSIS`
- 电容满电电压：`V_CAP_FULL`
- 低压保护阈值：`V_CAP_LOW_THRESHOLD`、`I_CAP_DISCHARGE_THRESHOLD`
- 过压保护阈值：直接写在 `HAL_callback.c:27`（`V_CAP_TF > 25.0f`）
- 掉电检测电压阈值：直接写在 `HAL_callback.c:94`（`chassis_voltage_window < 19.0f`）
- 掉电检测电流阈值：直接写在 `HAL_callback.c:89`（`I_CAP_TF < -0.2f && I_CHASSIS_TF < 0.3f`）

### ADC 校准与标定

每块板子有独立的线性校准参数，存储在 `ADC_CALIBRATION_CONFIGS[4][4][2]` 中：

- 第一维：板号（0~3，由 `ADC_ID_init()` 自动识别）
- 第二维：通道（0=V_CHASSIS, 1=I_CHASSIS, 2=I_CAP, 3=V_CAP）
- 第三维：`{scale, bias}` → `物理值 = raw_counts × scale + bias`

当前版本 `ADC_Transformer_voltage()` 和 `ADC_Transformer_current()` 使用硬编码公式，未调用校准表。如需启用板级校准，需修改转换函数使用 `ADC_CALIBRATION_CONFIGS_BOARD` 中的参数。

### 编码规范

- **缩进**：4 空格，无 Tab。
- **命名**：变量 `snake_case`，类型 `PascalCase`，常量 `UPPER_SNAKE_CASE`。
- **头文件**：仅包含声明（typedef、extern、函数原型），变量定义放在 `.c` 中。
- **include 防护**：使用 `#pragma once`。
- **回调轻量**：中断回调中避免阻塞、大量浮点运算、`printf` 等耗时操作。
- **常量集中**：硬件参数统一定义在 `const_data.c`，禁止在模块代码中硬编码魔法数字。
- **格式化**：VS Code 已配置保存时自动格式化（`settings.json:31`），风格近似 Google C++ Style（4 空格缩进）。

### 常用配置修改点速查

| 修改内容 | 文件位置 |
|----------|----------|
| 编译器 / 链接器标志 | `CMakeLists.txt:25-26`、`cmake/gcc-arm-none-eabi.cmake:10-26` |
| IntelliSense includePath | `.vscode/c_cpp_properties.json` |
| clangd 参数 | `.vscode/settings.json` |
| 外设初始化 | `Core/Src/main.c` |
| 外设引脚宏 (GPIO pins) | `Core/Inc/main.h` |
| 所有系统常量 | `User/data/const_data.c` |
| 保护判决逻辑 | `User/v1/src/HAL_callback.c` |
| PID 参数 | `User/v1/src/SuperCap_init.c` 中初始化调用 |
| 软启动逻辑 | `User/v1/src/SuperCap_init.c` |
| CAN 协议 & 断联逻辑 | `User/v1/src/CAN_communicate.c` |

---

## Git 工作流

- **主分支**：`9x60F`（当前开发分支）。
- **Commit 格式**：`<type>: <description>`，如 `feat(can): ...`、`refactor: ...`、`fix: ...`。
- **推送前整理**：使用 `git rebase -i` squash 调试 / 试错类提交，保持主分支历史干净。
- **误操作恢复**：`git reflog` 查看 HEAD 历史，`git reset --hard <hash>` 回到变基前状态。
- **远端仓库**：https://github.com/Alliance-Hardware/SuperCap_Control.git

---

## 调试技巧

- **观测关键变量**：在 J-Link 调试器中监视 `adc_data.V_CAP_TF`、`adc_data.I_CAP_TF`、`power_pid_configs.output`、`current_pid_configs.output`、`mos_driver.OUT_MIN` / `OUT_MAX` 等字段。
- **CAN 日志**：通过 CAN 分析仪（或 `can-utils`）监听 `RMCS_ID` 发的目标功率和使能指令，排查通信问题。
- **逻辑判断验证**：如怀疑某个 `if` 条件未生效（如之前 `I_CAP_ADC < -0.2f` 的 bug），可在对应分支内添加断点或 LED 翻转来验证。
- **详细设计文档**：https://fa4g5no1b1f.feishu.cn/wiki/AhU2wcN2ditQpXkVa6RcQfOXnyc
- **调试方法**：https://fa4g5no1b1f.feishu.cn/wiki/MsbMw05AsiN0lbkC2NecwQ2knag

---

## 维护建议

- `.h` 仅声明，`.c` 定义。头文件不要直接包含可执行代码或变量定义，避免多文件编译重定义。
- 硬件常数集中管理在 `const_data.c`，命名带单位前缀（`V_` 电压、`I_` 电流、`P_` 功率），方便移植到不同板型。
- `Core/` 目录由 CubeMX 生成，修改外设配置时重新生成覆盖即可；用户代码全在 `User/` 下，不受 CubeMX 覆盖影响。
- DSP / NN 库仅需在 CMake 中按需启用对应模块源文件，避免编译所有模块增加链接时间。
