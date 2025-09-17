#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout( push_constant ) uniform constants
{
	mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
} PushConstants;

void main() {
    gl_Position = PushConstants.projMatrix * PushConstants.viewMatrix * PushConstants.modelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
}