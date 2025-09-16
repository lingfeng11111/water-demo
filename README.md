# Asylum Ocean - 恐怖海浪渲染系统

一个专业的实时海浪渲染系统，营造阴暗诡异的精神病院氛围。使用现代OpenGL技术实现真实的水面效果，包括Gerstner波浪动画、法线贴图、反射折射和HDR环境光照。

## 项目特性

### 🌊 真实海浪渲染
- **Gerstner波浪算法**: 4层叠加波浪，创造自然的海面运动
- **动态顶点位移**: 实时计算波浪高度和法线
- **物理准确的波浪形状**: 包括波峰尖锐化和波谷平滑化

### 🎨 专业材质系统
- **多层法线贴图**: 混合两层法线贴图创造复杂水面细节
- **菲涅尔反射**: 基于视角的反射强度变化
- **折射效果**: 真实的水下光线折射
- **动态泡沫**: 基于波浪高度和法线的程序化泡沫生成

### 🌅 HDR环境系统
- **HDR天空盒**: 支持高动态范围环境贴图
- **球面映射**: 将HDR纹理正确映射到天空球
- **环境反射**: 水面实时反射天空环境

### 👻 恐怖氛围效果
- **阴暗色调**: 降低整体亮度营造压抑感
- **雾效系统**: 距离雾增强神秘感
- **精神病院色调**: 轻微绿色调模拟医院灯光
- **动态云层**: 程序化云层增加不安感

### 🎮 交互控制
- **自由相机**: WASD移动，鼠标视角控制
- **垂直移动**: 空格上升，Shift下降
- **流畅操作**: 60FPS+渲染，响应式控制

## 技术规格

- **图形API**: OpenGL 4.1 Core Profile
- **着色器语言**: GLSL 4.10
- **数学库**: GLM (OpenGL Mathematics)
- **窗口管理**: GLFW 3.x
- **扩展加载**: GLEW
- **纹理加载**: stb_image
- **抗锯齿**: 4x MSAA

## 系统要求

### 最低要求
- **操作系统**: macOS 10.12+ / Windows 10+ / Linux
- **显卡**: 支持OpenGL 4.1的独立显卡
- **内存**: 4GB RAM
- **存储**: 100MB可用空间

### 推荐配置
- **显卡**: GTX 1060 / RX 580 或更高
- **内存**: 8GB RAM
- **分辨率**: 1920x1080或更高

## 编译指南

### macOS (当前配置)

#### 前置依赖
```bash
# 安装Homebrew (如果未安装)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装GLEW和GLM
brew install glew glm

# 手动安装GLFW (已配置路径 /usr/local)
# 或使用 brew install glfw
```

#### 编译步骤
```bash
# 进入项目目录
cd /Users/lingfeng/Desktop/wave

# 创建构建目录
mkdir -p cmake-build-debug
cd cmake-build-debug

# 配置CMake
cmake ..

# 编译项目
make -j$(nproc)

# 运行程序
./wave
```

### Windows

#### 前置依赖
1. 安装Visual Studio 2019+
2. 安装vcpkg包管理器
3. 安装依赖包：
```cmd
vcpkg install glfw3:x64-windows
vcpkg install glew:x64-windows  
vcpkg install glm:x64-windows
```

#### 编译步骤
```cmd
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg路径]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

### Linux (Ubuntu/Debian)

#### 前置依赖
```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install libglfw3-dev libglew-dev libglm-dev
```

#### 编译步骤
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
./wave
```

## 资源文件

项目需要以下纹理资源（已包含在resources/目录）：

- `Water_001_COLOR.jpg` - 水面颜色贴图
- `Water_001_NORM.jpg` - 水面法线贴图  
- `Water_001_DISP.png` - 水面位移贴图
- `Water_001_OCC.jpg` - 水面遮挡贴图
- `Water_001_SPEC.jpg` - 水面高光贴图
- `sky.hdr` - HDR环境贴图

## 控制说明

### 相机控制
- **W/A/S/D** - 前后左右移动
- **空格** - 向上移动
- **Shift** - 向下移动
- **鼠标** - 视角旋转
- **ESC** - 退出程序

### 渲染参数
程序内置了精心调试的参数来营造恐怖氛围：
- 波浪强度和频率
- 反射折射比例
- 雾效密度
- 光照颜色和强度

## 项目结构

```
wave/
├── CMakeLists.txt          # CMake配置文件
├── wave.cpp                # 主程序源码
├── water_vertex.glsl       # 水面顶点着色器
├── water_fragment.glsl     # 水面片段着色器
├── skybox_vertex.glsl      # 天空盒顶点着色器
├── skybox_fragment.glsl    # 天空盒片段着色器
├── resources/              # 纹理资源目录
│   ├── Water_001_*.jpg     # 水面材质贴图
│   └── sky.hdr            # HDR环境贴图
└── README.md              # 项目说明文档
```

## 技术实现细节

### Gerstner波浪算法
```glsl
// 4层波浪叠加
Wave waves[4] = Wave[4](
    Wave(normalize(vec2(1.0, 0.3)), 0.8, 0.4, 0.5, 0.3),
    Wave(normalize(vec2(0.7, 0.9)), 0.6, 0.6, 1.2, 0.25),
    Wave(normalize(vec2(-0.5, 0.8)), 0.4, 0.8, 2.1, 0.2),
    Wave(normalize(vec2(-0.9, -0.2)), 0.3, 1.2, 0.8, 0.15)
);
```

### 恐怖氛围参数
```cpp
// 阴暗光照
lightColor = vec3(0.4f, 0.5f, 0.6f);
darknessIntensity = 0.4f;

// 雾效颜色（精神病院绿调）
fogColor = vec3(0.1f, 0.15f, 0.12f);
fogDensity = 0.02f;

// 水面色调
waterTint = vec3(0.1f, 0.3f, 0.4f);
```

## 性能优化

- **LOD系统**: 150x150网格分辨率平衡质量和性能
- **纹理压缩**: 使用mipmap减少带宽
- **着色器优化**: 精简计算避免分支
- **批量渲染**: 单次绘制调用渲染整个水面

## 故障排除

### 常见问题

**Q: 编译时找不到GLFW/GLEW**
A: 确保库文件安装在正确路径，检查CMakeLists.txt中的路径配置

**Q: 程序启动黑屏**
A: 检查显卡驱动是否支持OpenGL 4.1，更新到最新版本

**Q: 纹理加载失败**
A: 确保resources/目录存在且包含所有必需的纹理文件

**Q: 性能较低**
A: 降低水面网格分辨率或关闭MSAA抗锯齿

### 调试模式
编译时添加调试信息：
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## 扩展功能

### 可添加的特性
- **粒子系统**: 水花和雨滴效果
- **后处理**: 色彩分级和景深
- **音效系统**: 海浪声和环境音
- **天气系统**: 动态天空和光照变化
- **交互物体**: 漂浮物和碰撞检测

## 许可证

本项目仅供学习和技术展示使用。纹理资源版权归原作者所有。

## 作者

技术美术渲染专家 - 专注于实时渲染和视觉效果开发

---

*"在这片诡异的海域中，每一道波浪都诉说着被遗忘的故事..."*
