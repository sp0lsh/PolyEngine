#version 430 core

in vec2 vUV;

out vec4 fragColor;

uniform vec4 uAlbedoColor;

void main()
{
	vec4 albedo = uAlbedoColor;
    fragColor = albedo;
}