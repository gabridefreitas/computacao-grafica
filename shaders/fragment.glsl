#version 330 core
out vec4 FragColor;

in vec3 vNormal;
in vec3 vFragPos;
in vec3 vBarycentric;
in vec2 vTexCoords;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct PointLight {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	vec3 direction;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;
};

struct DirLight {
	vec3 direction;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

#define MAX_LIGHTS 4

uniform Material material;
uniform PointLight pointLights[MAX_LIGHTS];
uniform SpotLight spotLights[MAX_LIGHTS];
uniform DirLight dirLight;

uniform int pointLightSize;
uniform int spotLightSize;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform samplerCube skybox;

uniform bool wireframe;
uniform vec3 wireframeColor;
uniform float wireframeWidth;
uniform bool hasTexture;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	// Calculate distance to nearest edge
	float minDist = min(vBarycentric.x, min(vBarycentric.y, vBarycentric.z));

	// Draw wireframe only on the actual edges
	// Use a small threshold (0.01-0.05) and make sure it's exactly on edges
	if (minDist < wireframeWidth && wireframe) {
		FragColor = vec4(wireframeColor, 1.0);
	} else {

		vec3 norm = normalize(vNormal);
		vec3 viewDir = normalize(viewPos - vFragPos);

		vec3 result = calcDirLight(dirLight, norm, viewDir);

		for (int i = 0; i < pointLightSize; i++){
			result += calcPointLight(pointLights[i], norm, vFragPos, viewDir);
		}

		for (int i = 0; i < spotLightSize; i++){
			result += calcSpotLight(spotLights[i], norm, vFragPos, viewDir);
		}

		vec3 I = normalize(vFragPos - viewPos);
		vec3 R = reflect(I, norm);
		vec3 skyboxColor = texture(skybox, R).rgb;
		
		vec3 skyboxAmbient = skyboxColor * material.ambient;
		vec3 skyboxSpecular = skyboxColor * material.specular;
		vec3 skyboxDiffuse = skyboxColor * material.diffuse;

		vec3 skyboxResult = skyboxAmbient + skyboxSpecular + skyboxDiffuse;
		
		result = mix(result, skyboxResult, material.shininess);
		FragColor = vec4(result, 1.0);
	}
}

// calculates the color when using a directional light.
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
    // combine results
    vec3 ambient = light.ambient * material.ambient;
		vec3 combinedDiffuse;
		if (hasTexture) combinedDiffuse = material.diffuse * texture(texture_diffuse1, vTexCoords).rgb;
		else combinedDiffuse = material.diffuse;
    vec3 diffuse = light.diffuse * diff * combinedDiffuse;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * material.ambient;
		vec3 combinedDiffuse;
		if (hasTexture) combinedDiffuse = material.diffuse * texture(texture_diffuse1, vTexCoords).rgb;
		else combinedDiffuse = material.diffuse;
    vec3 diffuse = light.diffuse * diff * combinedDiffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess * 128);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * material.ambient;
		vec3 combinedDiffuse;
		if (hasTexture) combinedDiffuse = material.diffuse * texture(texture_diffuse1, vTexCoords).rgb;
		else combinedDiffuse = material.diffuse;
    vec3 diffuse = light.diffuse * diff * combinedDiffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}
