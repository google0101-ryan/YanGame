#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoords;
layout(location = 2) flat in uint albedoIndex;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform sampler2D textures[];

void main() {
    outColor = texture(textures[albedoIndex], texCoords);
}
