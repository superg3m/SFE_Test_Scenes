#version 330 core

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;
 
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;        
};

struct Material {
    sampler2D diffuse_map;
    sampler2D specular_map;

    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;
    
    float shininess;
    float opacity;
};

#define NR_POINT_LIGHTS 4

uniform Material uMaterial;
uniform vec3 uViewPosition;

uniform DirLight uDirectionalLight;
uniform PointLight uPointLights[NR_POINT_LIGHTS];
uniform SpotLight uSpotLight;

uniform bool uUseFlashlight; 
uniform bool uBlinnPhong;

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 FragColor;

vec3 CalculateDiffuseColor(vec3 N, vec3 L, vec3 color) {
    float lambertian_mask = max(dot(N, L), 0.0);
    return (lambertian_mask * color);
}

vec3 CalculateSpecularColor(vec3 N, vec3 L, vec3 V, vec3 color) {
    float specular_mask = 0.0;
    if(uBlinnPhong) {
        vec3 H = normalize(L + V);  
        specular_mask = pow(max(dot(N, H), 0.0), uMaterial.shininess);
    } else {
        vec3 R = reflect(-L, N);
        specular_mask = pow(max(dot(V, R), 0.0), uMaterial.shininess);
    }

    return (specular_mask * color);
}

vec3 CalcDirectionalLight(const DirLight light, vec3 diffuse_texel, vec3 specular_texel, const vec3 N, const vec3 V) {
    vec3 L = normalize(-light.direction);

    vec3 ambient_color = light.ambient * uMaterial.ambient_color * diffuse_texel;
    vec3 diffuse_color = light.diffuse * uMaterial.diffuse_color * diffuse_texel;
    vec3 specular_color = light.specular * uMaterial.specular_color * specular_texel;

    vec3 ambient = ambient_color;
    vec3 diffuse = max(dot(N, L), 0.0f) * diffuse_color;
    vec3 specular = CalculateSpecularColor(N, L, V, specular_color);

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(const PointLight light, vec3 diffuse_texel, vec3 specular_texel, const vec3 N, const vec3 V) {
    vec3 L = normalize(light.position - v_FragPos);

    vec3 ambient_color = light.ambient * uMaterial.ambient_color * diffuse_texel;
    vec3 diffuse_color = light.diffuse * uMaterial.diffuse_color * diffuse_texel;
    vec3 specular_color = light.specular * uMaterial.specular_color * specular_texel;

    float D = length(light.position - v_FragPos);
    float attenuation = 1.0f / (light.constant + light.linear * D + light.quadratic * (D * D));

    vec3 ambient = ambient_color * attenuation;
    vec3 diffuse = CalculateDiffuseColor(N, L, diffuse_color) * attenuation;
    vec3 specular = CalculateSpecularColor(N, L, V, specular_color) * attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(const SpotLight light, vec3 diffuse_texel, vec3 specular_texel, const vec3 N, const vec3 V) {
    vec3 L = normalize(light.position - v_FragPos);

    vec3 ambient_color = light.ambient * uMaterial.ambient_color * diffuse_texel;
    vec3 diffuse_color = light.diffuse * uMaterial.diffuse_color * diffuse_texel;
    vec3 specular_color = light.specular * uMaterial.specular_color * specular_texel;

    float D = length(light.position - v_FragPos);
    float attenuation = 1.0f / (light.constant + light.linear * D + light.quadratic * (D * D));

    float theta = dot(L, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);

    vec3 ambient = ambient_color * attenuation * intensity;
    vec3 diffuse = CalculateDiffuseColor(N, L, diffuse_color) * attenuation * intensity;
    vec3 specular = CalculateSpecularColor(N, L, V, specular_color) * attenuation * intensity;

    return (ambient + diffuse + specular);
}

void main() {     
    vec3 N = normalize(v_Normal);
    vec3 V = normalize(uViewPosition - v_FragPos);

    vec3 diffuse_texel = texture(uMaterial.diffuse_map, v_TexCoord).rgb;
    vec3 specular_texel = texture(uMaterial.specular_map, v_TexCoord).rgb;
    
    vec3 total_light_result = vec3(0.0f); 
    total_light_result += CalcDirectionalLight(uDirectionalLight, diffuse_texel, specular_texel, N, V);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        total_light_result += CalcPointLight(uPointLights[i], diffuse_texel, specular_texel, N, V);         
    }

    if (uUseFlashlight) {
        total_light_result += CalcSpotLight(uSpotLight, diffuse_texel, specular_texel, N, V);       
    }
    
    FragColor = vec4(total_light_result, uMaterial.opacity);
    // FragColor = vec4(v_Normal, 1.0);
}