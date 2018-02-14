#version 330 core

in vec3 vUV;

out vec4 color;

uniform samplerCube uCubemap;

void main()
{
    color = mix(vec4(2.0, 1.0, 1.0, 1.0), vec4(texture(uCubemap, vUV).rgb, 1.0), 0.5);
}