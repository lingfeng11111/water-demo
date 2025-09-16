# Asylum Ocean - 恐怖海浪渲染系统

一个基于现代 OpenGL 的实时海浪渲染与氛围营造项目

## 项目特性
- **实时海浪**: Gerstner 波 + 顶点位移，动态法线与多层流动纹理。
- **PBR风格要素**: 菲涅尔反射、镜面高光、环境辐射近似采样。
- **HDR天空**: 以 HDR 纹理驱动的天空穹顶，时间驱动动态云层调制。
- **恐怖氛围**: 低照度、绿色雾、色调偏移，压抑叙事氛围。
- **交互相机**: WASD/空格/Shift + 鼠标视角 + 滚轮视野。
- **跨平台**: macOS/Windows/Linux，OpenGL 4.1 Core Profile。
- **CMake构建**: 标准化依赖与资源复制流水线。
- **纹理资源**: 颜色、法线、置换、高光、HDR 天空纹理。
- **性能权衡**: 网格分辨率可调，简化分支与贴图使用策略。
- **可拓展性**: 后处理、粒子雨、天气系统、音频可插拔。

---

## 架构与实现

### 1. 总览
- **目标**: 在统一、清晰、可拓展的渲染框架下，实现具有恐怖氛围的海面渲染与天空光环境。
- **核心模块**:
  - `wave.cpp` 主程序：窗口/上下文创建、输入、资源加载、渲染循环、参数驱动。
  - 着色器：
    - `water_vertex.glsl`：Gerstner 波位移与法线近似，动态多层流动纹理坐标。
    - `water_fragment.glsl`：反射、Fresnel、高光、泡沫、雾效、色调与亮度控制。
    - `skybox_vertex.glsl / skybox_fragment.glsl`：HDR 天空穹顶采样与动态云层调制。
  - 构建系统：`CMakeLists.txt` 集中依赖解析、目标、资源拷贝。
  - 资源：`resources/` 目录存放水面贴图与 `sky.hdr`。
- **运行时数据流**（高层）:
  1) CPU 初始化 → OpenGL 状态 → 着色器编译/链接 → 纹理加载。
  2) CPU 生成水面网格 + 天空盒顶点 → 绑定 VAO/VBO/EBO。
  3) 循环内：输入 → 时间/相机矩阵 → 天空渲染 → 水面渲染 → 交换缓冲。

### 2. 依赖与第三方库
- OpenGL 4.1 Core Profile：跨平台图形 API。
- GLFW：跨平台窗口/上下文/输入。
- GLEW：OpenGL 扩展函数加载。
- GLM：线性代数与矩阵运算（仿 GLSL 接口）。
- stb_image：纹理与 HDR 图像加载。
- ZLIB：链接依赖（部分系统包依赖）。

### 3. 构建系统（CMake）
- 关键设置（参考 `CMakeLists.txt`）:
  - C++20 开启：`set(CMAKE_CXX_STANDARD 20)`。
  - `FetchContent` 引入 `stb` 源，避免手工维护副本。
  - 头文件与库目录（macOS/Homebrew 常见路径）：`/usr/local/include` 与 `/usr/local/lib`。
  - `find_package`：`GLEW`、`glm`、`ZLIB`。
  - 目标链接：`glfw`、`GLEW::GLEW`、`glm::glm`、`ZLIB::ZLIB`，以及 macOS 框架（`OpenGL`、`Cocoa`、`IOKit`、`CoreVideo`）。
  - 运行时资源复制：将着色器与 `resources/` 复制到构建目录，保证可执行文件的相对路径加载。
- 跨平台建议：
  - Windows 使用 vcpkg 工具链文件；Linux 使用包管理器安装 dev 头文件；macOS 建议使用 Homebrew。

### 4. 程序入口与生命周期（`wave.cpp`）
- 初始化阶段：
  - `glfwInit()` + 上下文要求（4.1 Core）+ `GLFW_OPENGL_FORWARD_COMPAT`（macOS 必需）。
  - 创建窗口与回调：视口变化、鼠标移动、滚轮。
  - GLEW 初始化；启用深度测试、混合；可按需启用/关闭 MSAA。
- 资源加载：
  - 着色器：`Shader waterShader("water_vertex.glsl", "water_fragment.glsl")`；`skyboxShader` 同理。
  - 纹理：
    - 水面：`waterColorTex`、`waterNormalTex`、`waterDispTex`、`waterSpecTex`。
    - 天空：`skyboxHDR`（HDR equirectangular）。
- 几何生成：
  - 水面网格：规则格点（默认 50×50），每顶点包含位置 (x,y,z) + 纹理坐标 (u,v)，EBO 建三角索引。
  - 天空盒：立方体 36 顶点三角集合。
- 绑定与布局：
  - 水面 VAO 绑定两条属性：location0 位置3浮点，location1 纹理坐标2浮点。
  - 天空盒 VAO 仅位置 3 浮点。
- Uniform 绑定：
  - 水面着色器：`waterColor/waterNormal/waterDisp/waterSpec/skyboxHDR` 绑定到 0..4 单元。
  - 天空着色器：`skyboxHDR` 绑定到 0 单元。
- 渲染循环：
  1) 更新时间差与累计时间。
  2) 处理输入（WASD/空格/Shift，鼠标与滚轮）。
  3) `glClear` 背景色（暗蓝近黑），清理颜色与深度。
  4) 计算 `model/view/projection`，其中 `projection` 使用 `fov` 可交互变焦。
  5) 渲染天空：`glDepthFunc(GL_LEQUAL)` + 去平移视图矩阵（仅保留旋转），防止摄像机位移影响天空。
  6) 渲染水面：设置一系列物理/氛围参数，绑定各纹理，`glDrawElements` 批量绘制。
  7) 交换缓冲并轮询事件。
- 终结：删除 VAO/VBO/EBO，销毁窗口，`glfwTerminate()`。

### 5. 相机与交互
- 位置/方向：`cameraPos/cameraFront/cameraUp`；`yaw/pitch` 通过鼠标更新。
- 视场角：`fov` 1°–45°，滚轮缩放。
- 输入绑定：
  - `W/S`: 前后；`A/D`: 左右（通过 `cross` 得到侧向向量）。
  - `Space/Left Shift`: 上下。
  - 鼠标移动：更新 `yaw/pitch` 并重建 `cameraFront`。
- 移动速度：与 `deltaTime` 相乘，保证不同帧率下一致性体验。

### 6. 水面网格生成（CPU）
- `generateWaterMesh(vertices, indices, resolution)`：
  - `size=50.0f`，`step=size/resolution`，生成 (resolution+1)^2 顶点。
  - 顶点布局：`[pos.x, pos.y(0), pos.z, uv.u, uv.v]`，将纹理坐标按比例放大（此处为 ×8）以引入重复细节。
  - 索引：每个栅格两三角，顺序为 `(topLeft, bottomLeft, topRight)` 与 `(topRight, bottomLeft, bottomRight)`，确保正面一致。
- 分辨率权衡：
  - 提高 `resolution` → 更平滑的位移与法线 → 更高的顶点/片元成本。
  - 对应优化策略见“性能优化”。

### 7. 天空盒构建（CPU）
- `generateSkyboxCube(vertices)`：
  - 立方体六面共 36 顶点（12 三角），仅位置。
  - 片元阶段将方向投影到球面坐标以采样 equirectangular HDR 纹理。

### 8. 资源加载（stb_image）
- LDR 纹理：`stbi_load`，判断 `nrComponents` → `GL_RED/GL_RGB/GL_RGBA`，上传后生成 mipmap；采样设置：`GL_REPEAT` + 线性过滤（mipmap）/线性放大。
- HDR 纹理：`stbi_loadf` 浮点读取；使用 `GL_RGB16F` 内部格式以保留动态范围；`CLAMP_TO_EDGE` 边界，线性过滤。
- 常见问题：
  - macOS 上 HDR equirectangular 方向可能出现翻转，片元着色器中 V 坐标做 1 - v 修正。
  - 资源路径：构建后执行目录依赖 CMake 的资源复制，若手动运行需确认工作目录。

### 9. 着色器系统

#### 9.1 顶点着色器：`water_vertex.glsl`
- 输入：位置与 UV。
- Uniform：`model/view/projection/time/viewPos`。
- Wave 结构：`direction(amplitude/frequency/phase/steepness)`。
- 波叠加：默认两层（可扩展至更多）；计算位移与法线近似项并叠加。
- 动画 UV：`WaveTexCoord1/2` 基于时间平移与尺度变换实现多层水流效果。
- 裁剪空间：输出用于后续反射/折射计算（当前反射近似不使用深度图/反射纹理，而采样天空）。

关键计算要点：
- 位移：
  - x/z 分量：`steepness * amplitude * direction.[x|y] * cos(phase)` → 模拟水平挤压。
  - y 分量：`amplitude * sin(phase)` → 波高。
- 法线近似：
  - 基于相位偏导的简单近似，叠加后归一化；对光照与反射方向计算足够稳定。

#### 9.2 片元着色器：`water_fragment.glsl`
- 采样：
  - `waterColor` 提供基色；`waterNormal` 提供法线扰动；`waterSpec` 控制高光强度；`skyboxHDR` 作为环境反射近似来源。
- 光照：
  - 使用视线方向 `viewDir`、入射 `lightDir` 与半角向量做 Blinn-Phong 风格高光。
- 反射与菲涅尔：
  - 反射方向 `reflect(-viewDir, normal)`；将方向映射为球面坐标(u,v)以从 equirectangular HDR 采样。
  - Fresnel 采用 `pow(1 - cosθ, 2)` 的简化近似，与 `reflectionStrength` 联合控制混合。
- 泡沫：
  - 基于世界空间高度 `worldPos.y` 与阈值做 `smoothstep`，产生波峰泡沫。
- 雾效与色调：
  - 指数衰减雾：`fogFactor=exp(-distance*fogDensity)`；通过 `mix(fogColor, color, fogFactor)` 混合。
  - `darknessIntensity` 统一控制亮度基线；`waterTint` 做轻度色调偏移。

#### 9.3 天空片元：`skybox_fragment.glsl`
- 将入射方向标准化后转球面坐标 (phi, theta)；V 轴翻转修正；采样 HDR。
- 动态云层：使用时间和方向分量的简单三角函数扰动，轻度混合 `fogColor`。
- 色调映射：简单 Reinhard 映射，防止高亮溢出。

### 10. 数学与坐标约定
- 空间：世界空间 → 观察空间（视图矩阵）→ 裁剪空间（投影矩阵）。
- 天空：视图矩阵去平移，仅保留旋转，使天空看似无限远。
- 波：相位 `phase = frequency * dot(direction, pos.xz) + phase * time`。
- Fresnel 简化：`pow(1 - dot(normal, viewDir), 2)`，工程上配合经验系数可达可接受效果。

### 11. 性能优化策略
- 网格分辨率：
  - 默认 50×50；在中端 GPU（如移动独显）可提高至 100×100 获得更平滑波形。
  - 低端设备可降至 30×30，并进一步减少波层数或关闭法线贴图。
- 贴图采样：
  - 简化为单法线层；必要时可叠加第二层并做旋转扰动以丰富细节。
- 分支与指令：
  - 着色器避免复杂分支；尽量用 `mix/smoothstep` 等连续函数近似。
- 批渲染：
  - 整个水面一次 DrawCall；如需多块水面可合并顶点缓冲并使用索引偏移。
- MSAA：
  - 关闭可显著改善性能；如镜面锯齿明显，可开启小倍率（例如 x2）。
- 纹理尺寸：
  - 视目标平台而定；PC 可用 2K/4K，移动设备建议 1K 左右并启用 mipmap。

### 12. 视觉一致性与数值建议
- 亮度与雾：
  - `darknessIntensity` 建议范围 [0.6, 1.2] 根据场景基准亮度微调。
  - `fogDensity` 建议 [0.003, 0.02]；越大雾感越重。
- 反射/折射：
  - 当前折射为占位（未实际采样屏幕/深度），可通过降低 `refractionStrength` 表达更偏反射的恐怖质感。
- 泡沫：
  - `foamThreshold` 控制泡沫出现高度；较高值使泡沫仅在波峰出现，视觉更克制。
- 色调：
  - `waterTint` 建议以青绿为主，轻量混合，避免过度染色导致材质塑料感。

### 13. 可扩展方向（技术路线）
- 真实反射/折射：
  - 通过帧缓冲捕获场景到反射/折射纹理；基于面法线做反射矢量采样；剪裁平面+二次渲染提升可信度。
- 法线合成：
  - 双法线贴图旋转流动 + 细节法线；TBN 空间重建进行切线空间正规扰动。
- PBR 合理性：
  - 基于 IBL 的反射模型：预计算向量映射或使用 cubemap；引入 roughness/metallic 近似控制水体粗糙度变化（沿岸更粗糙）。
- 泡沫增强：
  - 使用导数（`fwidth`）检测屏幕空间曲率/轮廓；或根据波相干叠加计算破碎波峰。
- 后处理：
  - 景深、胶片颗粒、色彩分级（LUT）与辉光来提升叙事氛围。
- 天气系统：
  - 程序化噪声驱动云层/风场；风向/风速与波向耦合。
- 音频与交互：
  - 波浪声、雷雨、风声；交互物（漂浮物）加入基于浮力的简单物理。

### 14. 目录结构
```
wave/
├── CMakeLists.txt
├── wave.cpp
├── water_vertex.glsl
├── water_fragment.glsl
├── skybox_vertex.glsl
├── skybox_fragment.glsl
├── resources/
│   ├── Water_001_COLOR.jpg
│   ├── Water_001_NORM.jpg
│   ├── Water_001_DISP.png
│   ├── Water_001_SPEC.jpg
│   └── sky.hdr
└── README.md
```

### 15. 关键代码片段（引用）

渲染循环（精简示意）：
```cpp
// 设置矩阵
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

// 天空
glDepthFunc(GL_LEQUAL);
skyboxShader.use();
skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
skyboxShader.setMat4("projection", projection);
// ... 绑定 skyboxHDR ...
// glDrawArrays(...)

// 水面
waterShader.use();
waterShader.setMat4("model", model);
waterShader.setMat4("view", view);
waterShader.setMat4("projection", projection);
waterShader.setFloat("time", currentFrame);
waterShader.setVec3("viewPos", cameraPos);
// ... 设置光照/雾/色调/强度 ...
// ... 绑定 waterColor/Normal/Disp/Spec/skyboxHDR ...
// glDrawElements(...)
```

Gerstner 波位移（顶点着色器思想）：
```glsl
vec3 calculateGerstnerWave(vec3 position, Wave w) {
    float phase = w.frequency * dot(w.direction, position.xz) + w.phase * time;
    return vec3(
        w.steepness * w.amplitude * w.direction.x * cos(phase),
        w.amplitude * sin(phase),
        w.steepness * w.amplitude * w.direction.y * cos(phase)
    );
}
```

### 16. 运行与编译

#### macOS
- 依赖安装（Homebrew）：`glew`、`glm`、`glfw`。
- 构建流程：
```bash
cd /Users/lingfeng/Desktop/wave
mkdir -p cmake-build-debug && cd cmake-build-debug
cmake ..
make -j$(sysctl -n hw.ncpu)
./wave
```
- 注意：若 `/usr/local` 非 Homebrew 路径，请依据本机环境修改 `include_directories` 与 `link_directories`。

#### Windows (MSVC + vcpkg)
```cmd
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libglew-dev libglm-dev
mkdir build && cd build
cmake ..
make -j$(nproc)
./wave
```

### 17. 参数目录（运行时可调建议）
- 光照：
  - `lightDir`: 建议偏斜上方入射，形成一侧高光。
  - `lightColor`: 略高于 1.0 的值可提亮暗部（当前已提高）。
- 水体：
  - `waterOpacity`: 0.7–0.9；恐怖氛围保持偏不透明。
  - `reflectionStrength`: 0.4–0.7；较强反射强化压迫感。
  - `refractionStrength`: 0.2–0.5；当前折射为近似，可保守使用。
  - `waterTint`: vec3(0.1, 0.3~0.4, 0.3) 区间，青绿。
  - `foamThreshold`: 0.4–0.7；越高泡沫越稀疏。
- 雾效：
  - `fogColor`: 偏绿与偏灰的平衡值增强医院冷感。
  - `fogDensity`: 0.003–0.02；场景尺度越大越可增加密度。
- 视角：
  - `fov`: 30°–45°；较小 FOV 更聚焦，压迫感增强。

### 18. 资源与路径约束
- 运行目录需能访问：
  - `resources/Water_001_COLOR.jpg`
  - `resources/Water_001_NORM.jpg`
  - `resources/Water_001_DISP.png`
  - `resources/Water_001_SPEC.jpg`
  - `resources/sky.hdr`
- CMake 在构建目录自动复制上述资源与着色器；手动运行时建议在可执行文件同级目录保留这些文件。

### 19. 质量与稳定性
- 着色器编译错误：
  - 构造 `Shader` 时输出编译/链接日志；若报错，请确认 GLSL 版本一致（410 core）。
- 驱动差异：
  - 不同平台对 equirectangular 映射采样细节略有差别；已在天空片元做 V 翻转修正。
- 兼容性：
  - 若 OpenGL 4.1 不可用，可尝试降级至 3.3 并改写少量接口（如布局限定与内置函数）—需额外适配。

### 20. 测试要点（人工巡检）
- 启动输出：OpenGL 版本、着色器与纹理加载日志应正常。
- 交互：鼠标移动视角连续、滚轮变焦平滑、键盘方向移动线性无抖动。
- 视觉：
  - 天空：亮度映射合理，云层动态轻微而不突兀。
  - 水面：反射随视角变化明显；高光不刺目；泡沫出现在波峰处。
  - 雾：远处明显偏绿，近处保留材质细节。
- 性能：
  - 1080p、50×50 网格应具备流畅帧率（依赖 GPU）。

### 21. 故障排除（FAQ）
- 编译找不到 GLFW/GLEW/GLM：
  - 确认包已安装；检查 CMake `find_package` 是否能解析；必要时加上 `CMAKE_PREFIX_PATH`。
- 程序启动黑屏：
  - 确认驱动支持 OpenGL 4.1；打印编译日志与 GL 错误；检查资源复制是否成功。
- 纹理加载失败：
  - 检查 `resources/` 目录文件名与路径；确认工作目录与 CMake 复制步骤。
- 帧率偏低：
  - 降低网格分辨率、关闭 MSAA、降低纹理分辨率，或减少波层与法线强度。
