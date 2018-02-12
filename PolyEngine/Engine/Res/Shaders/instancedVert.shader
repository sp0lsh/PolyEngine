#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 3) in mat4 aOffset;

out vec2 vTexCoord;

uniform mat4 uP;
uniform mat4 uMV;
uniform float uTime;

float nrand(float n)
{
    return fract(sin(dot(n, 12.9898)) * 43758.5453);
}

void main()
{
	float rnd = nrand(float(gl_InstanceID));
    float _ScaleY = aOffset[0][0];
    float _ScaleX = aOffset[1][1];

    vec4 p = vec4(aPos, 1.0);
//     p = uP * (uMV * aOffset * vec4(0.0, 0.0, 0.0, 1.0)
// 		+ vec4(p.x, p.y, 0.0, 0.0)
//         * vec4(_ScaleX, _ScaleY, 1.0, 1.0));
    p = uP * uMV * aOffset * p;
	
    gl_Position = p;
    vTexCoord = aTexCoord;
}