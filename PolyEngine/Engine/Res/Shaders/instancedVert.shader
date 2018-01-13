#version 330 core

// layout(location = 0) in vec4 aPos;
// layout(location = 1) in	vec2 aTexCoord;
// layout(location = 2) in vec3 aNormal;


layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aTexCoord;
layout(location = 3) in mat4 aOffset;

out vec3 vTexCoord;

uniform mat4 uMVP;
uniform float uTime;

float nrand(float n)
{
    return fract(sin(dot(n, 12.9898)) * 43758.5453);
}

void main()
{
    float rnd = nrand(float(gl_InstanceID));
    float phase = 0.5 * (1.0 + sin(rnd + 2.0 * uTime));
    vec4 p = uMVP * vec4(aPos, 1.0);
    // p.xyz *= 0.5;
    p = mix(p, aOffset * p, vec4(phase));
    gl_Position = p;

	vTexCoord = aTexCoord;
}