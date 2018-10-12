#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

uniform mat4 uClipFromModel;

void main() {
	gl_Position = uClipFromModel * vec4(aPosition, 1.0);
	vUV = aUV;
}