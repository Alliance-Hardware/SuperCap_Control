# SUPERCAP

基于 **STM32G4** 的超级电容管理与闭环控制项目，包含 CAN 通信、ADC 采样、MOS 驱动、PID 控制等模块。代码按功能分层，便于替换算法或移植外设，适合工程开发与学习参考。

## 目录结构
- `Core/`：STM32CubeMX 生成的基础工程（时钟、外设初始化、启动文件）
- `User/`：用户功能代码  
  - `data/`：常量与全局模块数据（参数、阈值、运行时状态）  
  - `interface/`：模块对外接口（供业务逻辑调用）  
  - `math_tools/`：数学与算法工具（滤波、标定、控制计算）  
  - `v1/src/`：业务逻辑与控制流程（主控制、CAN 回调、保护逻辑）
- `cmake/`：交叉编译工具链配置
- `build/`：构建输出目录（本地生成）

## 开发逻辑（模块协作）
- **采样链路**：ADC 采样 → 标定（`math_tools`）→ 写入 `module_data`  
- **控制链路**：控制回调中读取 `module_data` → PID 计算 → MOS 驱动输出  
- **通信链路**：CAN 接收 → 更新目标功率与状态 → CAN 发送状态数据  
- **保护链路**：过压/低压/断联/掉电检测 → 保护触发 → 停止输出与重置控制

> 设计原则：**数据集中管理（data）+ 接口解耦（interface）+ 业务逻辑统一调度（v1/src）**

## 控制逻辑（闭环流程）
1. **定时中断回调**触发采样更新  
2. **电压/电流标定**并写入 `module_data`  
3. **PID 计算**输出占空比（电流/功率环）  
4. **MOS 驱动**执行占空比更新  
5. **保护判断**（过压/低压/掉电/断联）  
6. 若触发保护 → **停止输出并重置 PID**

## 通信逻辑（CAN）
- **接收**：解析 CAN 数据帧，更新 `targetChassisPower` 与 `enabled`  
- **校验**：目标功率范围裁剪，异常使能值保护  
- **发送**：周期性广播当前电压、电流、功率等状态  
- **断联检测**：未收到 CAN 超时进入保护状态

## 功能概览
- CAN 通信（收发与状态检测）
- ADC 采样与标定
- 软启动与 MOS 驱动
- PID 电流/功率闭环
- 保护策略（过压/低压/断联/掉电检测）

## 依赖环境
- STM32 工具链（arm-none-eabi）
- CMake
- Ninja
- VS Code（推荐 CMake Tools 插件）

## 构建（Windows）
> 使用 PowerShell 终端

```bat
cmake --preset Debug
cmake --build --preset Debug
```

如果没有预设，可手动配置：

```bat
cmake -S . -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build -- -j 4
```

## 常见问题
### 1) 编译器检测失败
确保使用交叉编译工具链（`arm-none-eabi-gcc`），并启用：
```
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```

### 2) 使用了错误的编译器
构建日志中若出现 `cc.exe`（MSYS2），说明未使用交叉工具链。请改用预设或指定 `CMAKE_TOOLCHAIN_FILE`。

## 学习建议
- 从 `User/v1/src/HAL_callback.c` 和 `User/v1/src/CAN_communicate.c` 入手了解控制流程  
- `User/data/const_data.c` 为关键参数配置  
- `User/data/module_data.c` 为运行时状态与模块数据结构  

---
如需扩展（如替换控制算法或加入新传感器），建议新增模块并通过 `interface/` 与主逻辑解耦。

