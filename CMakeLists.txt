cmake_minimum_required(VERSION 3.24)
project(wave LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# 手动 glfw 头文件 & 库路径
include_directories(/usr/local/include)
include_directories(${stb_SOURCE_DIR})
link_directories(/usr/local/lib)

# 查找依赖包
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(ZLIB REQUIRED)

# 生成可执行文件
add_executable(wave
        wave.cpp)

# 链接库
target_link_libraries(wave
        glfw
        GLEW::GLEW
        glm::glm
        ZLIB::ZLIB
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo")

# 运行时搜索路径
target_link_directories(wave PRIVATE /usr/local/lib)
set_target_properties(wave PROPERTIES
        INSTALL_RPATH /usr/local/lib
        BUILD_WITH_INSTALL_RPATH TRUE)

# 复制资源文件到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# 复制着色器文件到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/water_vertex.glsl DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/water_fragment.glsl DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/skybox_vertex.glsl DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/skybox_fragment.glsl DESTINATION ${CMAKE_BINARY_DIR})
