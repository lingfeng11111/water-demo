#version 410 core

in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;
in vec3 WorldPos;
in vec4 ClipSpace;
in vec2 WaveTexCoord1;
in vec2 WaveTexCoord2;

out vec4 FragColor;

// 纹理
uniform sampler2D waterColor;
uniform sampler2D waterNormal;
uniform sampler2D waterDisp;
uniform sampler2D waterSpec;
uniform sampler2D skyboxHDR;

// 光照参数
uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform float time;

// 水面参数
uniform float waterOpacity;
uniform float reflectionStrength;
uniform float refractionStrength;
uniform vec3 waterTint;
uniform float foamThreshold;

// 恐怖氛围参数
uniform vec3 fogColor;
uniform float fogDensity;
uniform float darknessIntensity;

vec3 getNormalFromMap(vec2 texCoords) {
    // 简化法线贴图采样，只用一层
    vec3 normal1 = texture(waterNormal, WaveTexCoord1).rgb * 2.0 - 1.0;
    
    // 转换到世界空间（简化版本）
    vec3 worldNormal = normalize(Normal + normal1 * 0.2);
    return worldNormal;
}

vec3 calculateReflection(vec3 normal, vec3 viewDir) {
    vec3 reflectDir = reflect(-viewDir, normal);
    
    // 简化的天空盒采样
    float phi = atan(reflectDir.z, reflectDir.x);
    float theta = acos(reflectDir.y);
    vec2 skyUV = vec2(phi / (2.0 * 3.14159) + 0.5, theta / 3.14159);
    
    vec3 skyColor = texture(skyboxHDR, skyUV).rgb;
    
    // 提高反射亮度
    skyColor *= darknessIntensity * 0.8;
    
    return skyColor;
}

float calculateFresnel(vec3 normal, vec3 viewDir) {
    float cosTheta = max(dot(normal, viewDir), 0.0);
    float fresnel = pow(1.0 - cosTheta, 2.0); // 简化菲涅尔计算
    return clamp(fresnel, 0.0, 1.0);
}

vec3 calculateSpecular(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0); // 降低高光锐度
    
    // 采样高光贴图
    float specularMap = texture(waterSpec, WaveTexCoord1).r;
    
    return spec * lightColor * specularMap;
}

float calculateFoam(vec3 worldPos, vec3 normal) {
    // 简化泡沫计算
    float waveHeight = worldPos.y;
    float foam = smoothstep(foamThreshold - 0.1, foamThreshold + 0.1, waveHeight);
    
    return foam * 0.5;
}

void main() {
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDirection = normalize(-lightDir);
    
    // 获取法线
    vec3 normal = getNormalFromMap(TexCoord);
    
    // 简化的水面颜色计算
    vec3 waterBaseColor = texture(waterColor, TexCoord).rgb;
    vec3 reflection = calculateReflection(normal, viewDir);
    
    // 简化的菲涅尔混合
    float fresnel = calculateFresnel(normal, viewDir);
    vec3 waterColor = mix(waterBaseColor, reflection, fresnel * reflectionStrength * 0.5);
    
    // 计算高光
    vec3 specular = calculateSpecular(normal, viewDir, lightDirection, lightColor);
    waterColor += specular;
    
    // 计算泡沫
    float foam = calculateFoam(WorldPos, normal);
    vec3 foamColor = vec3(0.9, 0.95, 1.0) * darknessIntensity;
    waterColor = mix(waterColor, foamColor, foam);
    
    // 应用水面色调
    waterColor = mix(waterColor, waterTint, 0.2);
    
    // 简化的距离雾效
    float distance = length(viewPos - FragPos);
    float fogFactor = exp(-distance * fogDensity);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    // 混合雾色
    waterColor = mix(fogColor, waterColor, fogFactor);
    
    // 应用整体亮度
    waterColor *= darknessIntensity;
    
    // 最终颜色输出
    FragColor = vec4(waterColor, waterOpacity);
}
