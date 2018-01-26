#version 330 core

layout(location = 0) in vec4 aPos;
layout(location = 1) in	vec2 aTexCoord;
// layout(location = 2) in	vec3 aNormal;
layout(location = 2) in mat4 aOffset;

uniform mat4 uMVP;
uniform float uTime;

// out vec3 vVertexPos;
out	vec2 vTexCoord;

float nrand(float n)
{
	return fract(sin(dot(n, 12.9898)) * 43758.5453);
}

void main()
{
	float rnd = nrand(float(gl_InstanceID));
    float phase = abs(sin(rnd + 2.0 * uTime));

    vec4 p = aPos;

    // p = aOffset * p;
    // p += phase * vec4(0.0, 0.0, 2.0, 0.0);    
    // p += phase * vec4(aOffset[0][0], aOffset[0][1], aOffset[0][2], 0.0);	// moves in x by +1
    // p += phase * vec4(aOffset[1][0], aOffset[1][1], aOffset[1][2], 0.0);	// moves in z by +1
    // p += phase * vec4(aOffset[2][0], aOffset[2][1], aOffset[2][2], 0.0); // moves in y by +1 
    
    // p += phase * vec4(aOffset[0][0], aOffset[1][0], aOffset[2][0], 0.0); // moves in x by +1
    // p += phase * vec4(aOffset[0][1], aOffset[1][1], aOffset[2][1], 0.0); // moves in y by +1
    // p += phase * vec4(aOffset[0][2], aOffset[1][2], aOffset[2][2], 0.0); // moves in z by +1
    
	//	p = mix(p, aOffset * p, vec4(phase));

    mat4 tmp;
    tmp[0][0] = 1.0;
    tmp[1][1] = 1.0;
    tmp[2][2] = 1.0;
    tmp[3][3] = 1.0;
    tmp[0][3] = aOffset[0][3];
    // tmp[1][3] = aOffset[1][3];
    // tmp[2][3] = aOffset[2][3];
	p = mix(p, tmp * p, vec4(phase));
    
	// p = mix(p, uMVP * p, vec4(phase));

	p = uMVP * p;
    gl_Position = p;
    vTexCoord = aTexCoord;
    // vVertexPos = aPos.xyz;
}