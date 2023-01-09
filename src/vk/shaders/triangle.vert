#version 450

layout( push_constant ) uniform PushConstant
{
    vec4 data;
    // mat4 render_matrix;
    mat4 model;
    mat4 view;
    mat4 projection;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
// layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out mat4 fragModel;
// layout(locatuib = 2) out vec3 fragNormal;

void main()
{
    // gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    mat4 render_matrix = mvp.projection * mvp.view * mvp.model;
    gl_Position = render_matrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragPos = vec3(mvp.model * vec4(inPosition, 1.0));
    fragModel = mvp.model;
    // fragNormal = inNormal;
}