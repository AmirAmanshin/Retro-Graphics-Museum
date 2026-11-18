#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal;

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    int enabled;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float intensity;
};

#define NR_POINT_LIGHTS 4

uniform sampler2D texDiffuse;
uniform vec3 viewPos;
uniform SpotLight flashlight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform int useSolidColor;
uniform vec3 solidColor;
uniform float emissiveStrength;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    if (light.intensity <= 0.001) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient  = light.ambient * baseColor;
    vec3 diffuse  = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec * 0.35;

    return (ambient + diffuse + specular) * attenuation * light.intensity;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    if (light.enabled == 0) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient  = light.ambient * baseColor;
    vec3 diffuse  = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec * 0.45;

    return (ambient + (diffuse + specular) * intensity) * attenuation;
}

void main() {
    vec3 baseColor = (useSolidColor == 1) ? solidColor : texture(texDiffuse, TexCoords).rgb;

    if (emissiveStrength > 0.0) {
        FragColor = vec4(baseColor * emissiveStrength, 1.0);
        return;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Почти полная темнота без источников света
    vec3 result = vec3(0.018, 0.020, 0.024) * baseColor;

    result += CalcSpotLight(flashlight, norm, FragPos, viewDir, baseColor);
    for (int i = 0; i < NR_POINT_LIGHTS; ++i) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, baseColor);
    }

    float dist = length(viewPos - FragPos);
    float fogFactor = clamp((13.0 - dist) / (13.0 - 1.5), 0.0, 1.0);
    vec3 fogColor = vec3(0.008, 0.008, 0.012);
    result = mix(fogColor, result, fogFactor);

    FragColor = vec4(result, 1.0);
}