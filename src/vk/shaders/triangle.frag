#version 450

layout(binding = 0) uniform sampler2D texSampler;

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
  vec3 col = texture(texSampler, fragTexCoord).xyz;

  outColor = vec4(col, 1.0);

  // outColor = vec4(calculateScreenSpaceNormal(fragPos), 1.0);
}