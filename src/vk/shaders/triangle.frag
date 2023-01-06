#version 450

layout(binding = 0) uniform UniformBufferObject
{
  vec4 ambientLight;
  vec4 lightPos;
} ubo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

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
  float diff = max(dot(calculateScreenSpaceNormal(fragPos), lightDir), 0.0);
  vec3 diffuse = diff * ubo.ambientLight.xyz;
  vec3 col = (ambient+diffuse)*(texture(texSampler, fragTexCoord).xyz);

  // light source indication
  vec4 lightIndicator = vec4(vec3(1.0, 0.0, 0.0), distance(ubo.lightPos.xyz, fragPos));
  col = mix(col, lightIndicator.xyz, 1.0-clamp(lightIndicator.w, 0.0, 1.0));

  outColor = vec4(col, 1.0);

  // outColor = vec4(calculateScreenSpaceNormal(fragPos), 1.0);
}