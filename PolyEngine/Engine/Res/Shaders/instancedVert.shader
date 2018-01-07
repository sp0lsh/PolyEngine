#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aTexCoord;
layout(location = 2) in vec2 aOffset;

out vec3 vTexCoord;

uniform float uTime;

void main(){
    gl_Position = vec4(vec3(0.09) * aPos + abs(0.5 * (1.0 + sin(uTime))) * vec3(aOffset, 0.0), 1.0f);
	vTexCoord = aTexCoord;
}