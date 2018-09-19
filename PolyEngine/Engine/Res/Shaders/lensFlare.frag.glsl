#version 430 core

in vec2 vUV;

uniform sampler2D uImage;

uniform float uTime;
uniform float uLensFlareScale;
uniform float uBrightThreshold;

out vec4 oColor;

float nrand(vec2 n)
{
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

vec3 flare()
{	
	float brightThreshold = 1.0;
    float sizeX = 0.3;
    float sizeY = 0.005;
	vec3 tint = vec3(0.1, 0.1, 1.0);

	vec3 base = texture(uImage, vUV).rgb;
	const int maxSamples = 10;
    float weight = 1.0;
    vec3 sum = vec3(0.0);
    for (int i=0; i<maxSamples; ++i)
    {		
        float rndX = nrand(vUV.xy+vec2(i)+0.1*uTime);
        float rndY = nrand(vUV.yx+vec2(i)+0.1*uTime);
		vec2 offset = vec2(mix(-sizeX, sizeX, rndX), mix(-sizeY, sizeY, rndY) );
		
		vec3 inputTex = texture(uImage, vUV + offset ).rgb;
		float luminance = dot(inputTex, vec3(0.2126, 0.7152, 0.0722));
		vec3 brighter = mix(vec3(0.0), inputTex, step(brightThreshold, luminance));
        weight += 1.0;
		
        sum += brighter;
	}
    
    return base + tint*sum/weight;
}

void main()
{
    vec3 color = flare();
    oColor = vec4(color, 1.0);
}

/*
vec3 flare(vec2 vUV)
{	        
	float brightThreshold = 0.8;
    float scale = 1.0;
    vec3 tint = vec3(0.5, 0.7, 1.0);
    float sizeX = 0.3;
    float sizeY = 0.005;
	
    vec3 base = texture(iChannel0, vUV).rgb;
	const int maxSamples = 20;
    float weight = 1.0;
    vec3 sum = vec3(0.0);
    for (int i=0; i<maxSamples; ++i)
    {		
        float rndX = nrand(vUV.xy+vec2(i)+0.1*iTime);
        float rndY = nrand(vUV.yx+vec2(i)+0.1*iTime);
		vec2 offset = scale * vec2(mix(-sizeX, sizeX, rndX), mix(-sizeY, sizeY, rndY) );
		
		vec3 inputTex = texture(iChannel0, vUV + offset ).rgb;
		float luminance = dot(inputTex, vec3(0.2126, 0.7152, 0.0722));
		vec3 brighter = mix(vec3(0.0), inputTex, step(brightThreshold, luminance));
        weight += 0.1;
		
		// rgb = max(rgb, brighter);
        sum += brighter;
	}
    
    return base + tint*sum/weight;
}

*/