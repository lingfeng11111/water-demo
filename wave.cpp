#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// 窗口尺寸
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// 相机参数
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.2f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = -10.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float fov = 45.0f; // 添加视野角度变量用于缩放

// 时间变量
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 着色器类
class Shader {
public:
    unsigned int ID;
    
    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        
        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            
            vShaderFile.close();
            fShaderFile.close();
            
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        
        unsigned int vertex, fragment;
        
        // 顶点着色器
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        
        // 片段着色器
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        
        // 着色器程序
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    
    void use() {
        glUseProgram(ID);
    }
    
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

private:
    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << std::endl;
            }
        }
    }
};

// 纹理加载函数
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    
    return textureID;
}

// HDR纹理加载函数
unsigned int loadHDR(const char* path) {
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float *data = stbi_loadf(path, &width, &height, &nrComponents, 0);
    unsigned int hdrTexture = 0;
    if (data) {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    }
    else {
        std::cout << "Failed to load HDR image." << std::endl;
    }
    
    return hdrTexture;
}

// 生成水面网格
void generateWaterMesh(std::vector<float>& vertices, std::vector<unsigned int>& indices, int resolution = 50) {
    vertices.clear();
    indices.clear();
    
    float size = 50.0f;
    float step = size / resolution;
    
    // 生成顶点
    for (int z = 0; z <= resolution; z++) {
        for (int x = 0; x <= resolution; x++) {
            float xPos = (x - resolution / 2.0f) * step;
            float zPos = (z - resolution / 2.0f) * step;
            
            // 位置
            vertices.push_back(xPos);
            vertices.push_back(0.0f);
            vertices.push_back(zPos);
            
            // 纹理坐标
            vertices.push_back((float)x / resolution * 8.0f); // 重复纹理
            vertices.push_back((float)z / resolution * 8.0f);
        }
    }
    
    // 生成索引
    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {
            int topLeft = z * (resolution + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (resolution + 1) + x;
            int bottomRight = bottomLeft + 1;
            
            // 第一个三角形
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            // 第二个三角形
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

// 生成天空盒立方体
void generateSkyboxCube(std::vector<float>& vertices) {
    vertices = {
        // 位置坐标
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
}

// 鼠标回调函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    yaw += xoffset;
    pitch += yoffset;
    
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// 鼠标滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

// 键盘输入处理
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    float cameraSpeed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
}

int main() {
    // 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_SAMPLES, 4); // 关闭MSAA提升性能
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Asylum Ocean - Horror Wave Simulation", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    });
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback); // 添加鼠标滚轮回调函数
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // 初始化GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // 配置全局OpenGL状态
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_MULTISAMPLE); // 关闭多重采样提升性能
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Loading shaders..." << std::endl;
    
    // 加载着色器 - 从构建目录读取
    Shader waterShader("water_vertex.glsl", "water_fragment.glsl");
    Shader skyboxShader("skybox_vertex.glsl", "skybox_fragment.glsl");
    
    std::cout << "Loading textures..." << std::endl;
    
    // 加载纹理
    unsigned int waterColorTex = loadTexture("resources/Water_001_COLOR.jpg");
    unsigned int waterNormalTex = loadTexture("resources/Water_001_NORM.jpg");
    unsigned int waterDispTex = loadTexture("resources/Water_001_DISP.png");
    unsigned int waterSpecTex = loadTexture("resources/Water_001_SPEC.jpg");
    unsigned int skyboxHDR = loadHDR("resources/sky.hdr");
    
    std::cout << "Textures loaded successfully!" << std::endl;
    std::cout << "Generating meshes..." << std::endl;
    
    // 生成水面网格
    std::vector<float> waterVertices;
    std::vector<unsigned int> waterIndices;
    generateWaterMesh(waterVertices, waterIndices, 50);
    
    // 生成天空盒网格
    std::vector<float> skyboxVertices;
    generateSkyboxCube(skyboxVertices);
    
    std::cout << "Water mesh: " << waterVertices.size()/5 << " vertices, " << waterIndices.size()/3 << " triangles" << std::endl;
    
    // 设置水面VAO
    unsigned int waterVAO, waterVBO, waterEBO;
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
    glGenBuffers(1, &waterEBO);
    
    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(float), waterVertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterIndices.size() * sizeof(unsigned int), waterIndices.data(), GL_STATIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 设置天空盒VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), skyboxVertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 设置着色器纹理单元
    waterShader.use();
    waterShader.setInt("waterColor", 0);
    waterShader.setInt("waterNormal", 1);
    waterShader.setInt("waterDisp", 2);
    waterShader.setInt("waterSpec", 3);
    waterShader.setInt("skyboxHDR", 4);
    
    skyboxShader.use();
    skyboxShader.setInt("skyboxHDR", 0);
    
    std::cout << "Starting render loop..." << std::endl;
    
    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 时间逻辑
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // 输入
        processInput(window);
        
        // 渲染
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f); // 阴暗的背景色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 设置变换矩阵
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        
        // 渲染天空盒
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // 移除平移
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setFloat("time", currentFrame);
        skyboxShader.setFloat("darknessIntensity", 1.0f); // 大幅提高天空盒亮度
        skyboxShader.setVec3("fogColor", glm::vec3(0.3f, 0.35f, 0.32f)); // 提高雾色亮度
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skyboxHDR);
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);
        
        // 渲染水面
        waterShader.use();
        waterShader.setMat4("model", model);
        waterShader.setMat4("view", view);
        waterShader.setMat4("projection", projection);
        waterShader.setFloat("time", currentFrame);
        waterShader.setVec3("viewPos", cameraPos);
        
        // 光照参数（阴暗恐怖氛围）
        waterShader.setVec3("lightDir", glm::vec3(0.3f, -0.8f, 0.5f));
        waterShader.setVec3("lightColor", glm::vec3(1.2f, 1.3f, 1.4f)); // 大幅提高光照亮度
        
        // 水面参数
        waterShader.setFloat("waterOpacity", 0.85f);
        waterShader.setFloat("reflectionStrength", 0.6f);
        waterShader.setFloat("refractionStrength", 0.4f);
        waterShader.setVec3("waterTint", glm::vec3(0.1f, 0.4f, 0.3f)); // 恢复绿色调
        waterShader.setFloat("foamThreshold", 0.5f);
        
        // 恐怖氛围参数
        waterShader.setVec3("fogColor", glm::vec3(0.2f, 0.3f, 0.25f)); // 绿色雾效
        waterShader.setFloat("fogDensity", 0.005f); // 大幅减少雾效密度
        waterShader.setFloat("darknessIntensity", 1.0f); // 大幅提高水面亮度
        
        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waterColorTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, waterNormalTex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, waterDispTex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, waterSpecTex);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, skyboxHDR);
        
        // 渲染水面
        glBindVertexArray(waterVAO);
        glDrawElements(GL_TRIANGLES, waterIndices.size(), GL_UNSIGNED_INT, 0);
        
        // 交换缓冲区和轮询IO事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 清理资源
    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
    glDeleteBuffers(1, &waterEBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    
    glfwTerminate();
    return 0;
}