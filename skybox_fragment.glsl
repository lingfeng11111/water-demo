#version 410 core

in vec3 TexCoords;
out vec4 FragColor;

uniform sampler2D skyboxHDR;
uniform float time;
uniform float darknessIntensity;
uniform vec3 fogColor;

void main() {
    // 将3D方向转换为球面坐标 - 修复上下颠倒
    vec3 direction = normalize(TexCoords);
    float phi = atan(direction.z, direction.x);
    float theta = acos(direction.y); // 恢复原来的映射
    vec2 uv = vec2(phi / (2.0 * 3.14159) + 0.5, 1.0 - theta / 3.14159); // 翻转V坐标
    
    // 采样HDR天空盒
    vec3 color = texture(skyboxHDR, uv).rgb;
    
    // 大幅提高整体亮度
    color *= darknessIntensity * 1.5;
    
    // 添加动态云层效果
    float cloudNoise = sin(time * 0.1 + direction.x * 2.0) * sin(time * 0.15 + direction.z * 1.5);
    color = mix(color, fogColor, cloudNoise * 0.1 + 0.05); // 进一步减少云层遮挡
    
    // 减少绿色调强度
    color = mix(color, color * vec3(0.98, 1.0, 0.99), 0.05);
    
    // 色调映射（简单的Reinhard）
    color = color / (color + vec3(1.0));
    
    FragColor = vec4(color, 1.0);
}
