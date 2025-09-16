#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;
out vec3 WorldPos;
out vec4 ClipSpace;
out vec2 WaveTexCoord1;
out vec2 WaveTexCoord2;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 viewPos;

// Gerstner波参数 - 简化为2层
struct Wave {
    vec2 direction;
    float amplitude;
    float frequency;
    float phase;
    float steepness;
};

// 减少到2个波浪提升性能
Wave waves[2] = Wave[2](
    Wave(normalize(vec2(1.0, 0.3)), 1.0, 0.4, 0.5, 0.3),
    Wave(normalize(vec2(-0.7, 0.9)), 0.7, 0.6, 1.2, 0.25)
);

vec3 calculateGerstnerWave(vec3 position, Wave wave) {
    float dotProduct = dot(wave.direction, position.xz);
    float phase = wave.frequency * dotProduct + wave.phase * time;
    float amplitude = wave.amplitude;
    float steepness = wave.steepness;
    
    // Gerstner波位移
    vec3 displacement = vec3(0.0);
    displacement.x = steepness * amplitude * wave.direction.x * cos(phase);
    displacement.z = steepness * amplitude * wave.direction.y * cos(phase);
    displacement.y = amplitude * sin(phase);
    
    return displacement;
}

vec3 calculateGerstnerNormal(vec3 position, Wave wave) {
    float dotProduct = dot(wave.direction, position.xz);
    float phase = wave.frequency * dotProduct + wave.phase * time;
    float amplitude = wave.amplitude;
    float steepness = wave.steepness;
    float frequency = wave.frequency;
    
    // 计算法线
    float dPhaseDx = frequency * wave.direction.x;
    float dPhaseDz = frequency * wave.direction.y;
    
    vec3 normal = vec3(0.0, 1.0, 0.0);
    normal.x -= dPhaseDx * steepness * amplitude * sin(phase);
    normal.z -= dPhaseDz * steepness * amplitude * sin(phase);
    normal.y -= frequency * amplitude * cos(phase);
    
    return normal;
}

void main() {
    vec3 worldPos = (model * vec4(aPos, 1.0)).xyz;
    
    // 应用2个Gerstner波（减少计算量）
    vec3 totalDisplacement = vec3(0.0);
    vec3 totalNormal = vec3(0.0, 1.0, 0.0);
    
    for(int i = 0; i < 2; i++) {
        totalDisplacement += calculateGerstnerWave(worldPos, waves[i]);
        totalNormal += calculateGerstnerNormal(worldPos, waves[i]);
    }
    
    // 应用位移
    worldPos += totalDisplacement;
    
    // 归一化法线
    totalNormal = normalize(totalNormal);
    
    // 输出变量
    FragPos = worldPos;
    WorldPos = worldPos;
    Normal = totalNormal;
    TexCoord = aTexCoord;
    
    // 创建动态纹理坐标用于水面动画
    WaveTexCoord1 = aTexCoord + vec2(time * 0.02, time * 0.01);
    WaveTexCoord2 = aTexCoord * 2.0 + vec2(-time * 0.015, time * 0.025);
    
    // 计算裁剪空间坐标用于反射/折射
    ClipSpace = projection * view * vec4(worldPos, 1.0);
    
    gl_Position = ClipSpace;
}
