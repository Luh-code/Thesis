#version 450

layout(binding = 0) uniform UniformBufferObject
{
  vec4 ambientLight;
  vec4 lightPos;
  vec4 specular;
  vec4 cameraPos;
} ubo;

layout(binding = 1) uniform WorldEnviorment
{
  vec3 ambientColor;
  float  ambientStrength;
} worldEnv;

layout(binding = 2) uniform Material
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
} mat;

layout(binding = 3) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in mat4 fragModel;

layout(location = 0) out vec4 outColor;

vec3 calculateScreenSpaceNormal(vec3 p)
{
  vec3 dx = dFdx(p);
  vec3 dy = -dFdy(p);
  return normalize(cross(dx, dy));
}

void main()
{
  vec3 ambient = ubo.ambientLight.w * ubo.ambientLight.xyz;
  vec3 lightDir = normalize(ubo.lightPos.xyz-fragPos);
  
  // mat3 modelTranspositionMatrix = mat3(transpose(inverse(fragModel)));
  vec3 normal = calculateScreenSpaceNormal(fragPos);

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * ubo.ambientLight.xyz;

  vec3 viewDir = normalize(ubo.cameraPos.xyz - fragPos);
  vec3 refDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, refDir), 0.0), ubo.specular.x);
  vec3 specular = (ubo.specular.y * spec * ubo.ambientLight.xyz);
  
  vec3 col = (ambient+diffuse+specular)*(texture(texSampler, fragTexCoord).xyz);

  // light source indication
  vec4 lightIndicator = vec4(vec3(1.0, 0.0, 0.0), distance(ubo.lightPos.xyz, fragPos));
  col = mix(col, lightIndicator.xyz, 1.0-clamp(lightIndicator.w, 0.0, 1.0));

  outColor = vec4(col, 1.0);

  // outColor = vec4(calculateScreenSpaceNormal(fragPos), 1.0);
}