#version 430 core

in vec2 vUV;

out vec4 fragColor;

uniform sampler2D uAlbedoTexture;
uniform vec4 uAlbedoColor;

void main()
{
	vec4 albedo = texture(uAlbedoTexture, vUV) * uAlbedoColor;
    fragColor = albedo;
}