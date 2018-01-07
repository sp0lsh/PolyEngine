#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aTexCoord;
layout(location = 2) in mat4 aOffset;

// attribute mat4 aOffset;

out vec3 vTexCoord;

uniform float uTime;


float nrand(float n)
{
    return fract(sin(dot(n, 12.9898)) * 43758.5453);
}

void main(){
    float rnd = nrand(float(gl_InstanceID));
    vec4 p = vec4(aPos, 1.0);
    p.xyz *= 0.09;
    // gl_Position = mix(p, aOffset * p, vec4(0.5 * (1.0 + sin(uTime))));
    gl_Position = mix(p, p + vec4(aOffset[0][3], aOffset[1][3], 0.0, 1.0), vec4(0.5 * (1.0 + sin(rnd+uTime))));
    // gl_Position = vec4(vec3(0.09) * aPos + abs(0.5 * (1.0 + sin(uTime))) * vec3(aOffset[0][3], aOffset[1][3], 0.0), 1.0f);
	vTexCoord = aTexCoord;
}