# 🎥 DreamRawStream

一个基于 **C++23** 的多路实时视频监控/流媒体系统：服务端采集摄像头画面，经 **H.264 / AAC** 硬核编码后通过自研网络库 **DreamNet** 推流，客户端用 **Qt6 + FFmpeg + OpenGL** 实现低延迟的多画面（九宫格）实时解码与播放。

---

## 📖 项目简介

DreamRawStream 分为三个部分：

| 模块 | 说明 |
|------|------|
| `streamServer` | 采集摄像头（Linux V4L2）→ x264 编码 H.264、fdk-aac 编码 AAC → DreamNet TCP 推流 |
| `streamClient` | Qt6 图形界面，九宫格最多同时观看 9 路流，FFmpeg 解码 + OpenGL 渲染 |
| `DreamNet`（子模块） | 自研的高性能网络库，基于 epoll 事件循环，提供 TCP Server / Client |

整体数据流：

```
摄像头(V4L2) ──> H.264/x264 + AAC/fdk-aac 编码 ──> DreamNet TCP 推流(端口 11451)
                                                          │
                                          DreamNet TCP 连接 ──> FFmpeg 解码 ──> OpenGL 渲染
```

## ✨ 项目特点

- 🚀 **低延迟**：x264 使用 `zerolatency` 预设，配合内存池与并发队列，减少拷贝与分配开销
- 🖥️ **多路播放**：客户端九宫格布局，最多同时连接 9 路服务器画面，点击切换选中
- 🧵 **多线程架构**：采集、编码、推流、解码、渲染解耦，`DreamThread` / `ConcurrentQueue` 自研基础设施
- 🎨 **OpenGL 硬件渲染**：自定义 YUV→RGB 着色器，画面实时上屏，选中高亮边框
- 🧠 **内存池复用**：`memoryPool` / `FFmpegPool` 复用缓冲，避免频繁内存分配
- 🔉 **音视频同步**：支持 AAC 音频采集、编码与播放（默认关闭，见「使用方法」）
- 🧩 **模块化设计**：`DataHandler` 责任链式数据传递，解码器与 sink 解耦

## 📁 目录结构

```
DreamRawStream
├── common/            # 公共基础组件（内存池、并发队列、数据处理器、H264/AAC 解析器等）
├── streamServer/      # 服务端（摄像头采集 + 编码 + 推流）
│   ├── include/camera # 摄像头 V4L2 采集、x264 / fdk-aac 编码器
│   ├── include/server # 推流服务器
│   └── src/
├── streamClient/      # 客户端（Qt6 界面 + FFmpeg 解码 + OpenGL 渲染）
│   ├── include/client # 解码调度器、流客户端
│   ├── include/decoder# 视频 / 音频解码器
│   ├── include/widget # 主窗口、视频渲染窗口
│   ├── shader/        # OpenGL 着色器
│   └── forms/         # Qt Designer 界面文件
├── third/src/         # 第三方源码（子模块：DreamNet、FFmpeg）
├── build_dep.sh       # 一键编译依赖（DreamNet + FFmpeg，Linux）
└── CMakeLists.txt
```

## 🧩 依赖

- **CMake** ≥ 3.21
- **编译器**：支持 C++23（GCC ≥ 13 / Clang ≥ 16 / MSVC 较新版本）
- **Qt6** ≥ 6.5（`Core`、`Widgets`、`OpenGLWidgets`、`Multimedia`）
- **FFmpeg**（以子模块方式提供，已按需配置编译）
- **x264**、**fdk-aac**、**x265**（H.264 / AAC / H.265 编码）
- **ALSA**（Linux 音频采集，仅服务端需要）
- **DreamNet**（子模块，自研网络库）

> 服务端的摄像头采集依赖 Linux 的 **V4L2**，因此 `streamServer` 仅在 Linux 上可用；`streamClient` 基于 Qt6 + FFmpeg，可跨平台。

---

## 🚀 编译

### 🐧 Linux

1. **克隆仓库（含子模块）**

```bash
git clone --recursive https://github.com/Sora-wu/DreamRawStream.git
cd DreamRawStream
```

> 若已克隆但缺子模块，执行：`git submodule update --init --recursive`

2. **安装系统依赖**（以 Debian / Ubuntu 为例）

```bash
sudo apt install build-essential cmake \
    libasound2-dev libfdk-aac-dev libx264-dev libx265-dev \
    qt6-base-dev qt6-multimedia-dev libgl1-mesa-dev
```

3. **编译第三方依赖**（DreamNet + FFmpeg，安装到 `third/build/linux`）

```bash
./build_dep.sh
```

4. **编译项目**

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

编译产物位于 `build/streamServer/streamServer` 与 `build/streamClient/streamClient`。

### 🪟 Windows

> ⚠️ 由于服务端依赖 Linux 的 V4L2 摄像头采集接口，Windows 上通常**只编译客户端 `streamClient`**。

1. **克隆仓库（含子模块）**，同 Linux 步骤 1。

2. **准备依赖**：
   - 安装 **Visual Studio**（含 C++ 工具链，支持 C++23）与 **CMake**
   - 安装 **Qt 6.5+**（含 `Multimedia` 组件），并确保 CMake 能找到（设置 `CMAKE_PREFIX_PATH`）
   - 编译 **FFmpeg** 并安装到 `third/build/win`（可使用 MSYS2 + MinGW 或 vcpkg，参照 `build_dep.sh` 中的 configure 参数，将 `--prefix` 指向 `third/build/win`）

3. **配置并编译**

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.5.3\msvc2022_64
cmake --build build --config Release
```

> CMake 会根据平台自动选择 `third/build/win` 下的 FFmpeg 头文件与库，因此务必保证该目录结构与 Linux 版一致（`include/`、`lib/`）。

---

## 🎮 使用方法

### 启动服务端（Linux）

服务端接收摄像头设备路径作为参数，默认监听端口 `11451`：

```bash
./build/streamServer/streamServer /dev/video0
```

若你有多个摄像头，可启动多个服务端实例（不同设备路径即可）。

### 启动客户端

```bash
./build/streamClient/streamClient
```

客户端打开后是一个九宫格监控界面：

1. 在左侧输入框输入服务端地址，格式为 `IP:端口`，例如 `127.0.0.1:11451`，回车连接
2. 画面会出现在当前选中的格子中；**点击任意格子可切换选中**，被选中的格子带高亮边框
3. 选中不同格子后输入不同地址，即可在同一窗口同时观看**多路**画面（最多 9 路）
4. 再次点击某格可查看该路已连接的地址

### 🔉 关于音频

音频播放默认是关闭的（多个画面同时出声会互相干扰）。如需开启，编辑 [streamClient/src/main.cpp](streamClient/src/main.cpp) 中 `main` 函数里被注释的音频初始化代码：

```cpp
AudioBufferDevice audioBufferDevice{};
std::unique_ptr<QAudioSink> sink = setupAudio(scheduler, audioBufferDevice);
```

> 当前默认只播放「选中格子」那一路的音频。

---

## ⚠️ 注意事项

- **Linux / Wayland** 环境下，`streamClient` 需要设置环境变量使用 X11 后端，否则可能无法显示：

```bash
export QT_QPA_PLATFORM=xcb
./build/streamClient/streamClient
```

- 服务端依赖系统安装的 `x264`、`fdk-aac`、`ALSA`，请确保编译前已安装对应开发包。

---

## 🔗 DreamNet

[DreamNet](https://github.com/Sora-wu/DreamNet) 是本项目同作者（Sora-wu）开发的 C++23 高性能网络库，基于 epoll 事件循环，提供 TCP Server / Client、Buffer、内存池等基础设施，作为子模块被本项目的服务端推流与客户端拉流所使用。欢迎一并关注与使用～

---

## 📄 License

本项目及其子模块 [DreamNet](https://github.com/Sora-wu/DreamNet) 的许可请参见各自仓库内的 LICENSE 文件。
