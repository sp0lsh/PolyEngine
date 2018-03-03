#version 330 core

uniform sampler2D i_color;
uniform float uTime;
uniform vec2 uResolution;

in vec2 vTexCoord;
out vec4 color;

uniform int uUseCashetes;
uniform float uDistortionPower;
uniform float uColorTempValue; // default 6500 in [1000.0, 40000.0]
uniform float uColorTempPower;
uniform float uColorTempLuminancePreservation;
uniform float uSaturationPower;
uniform float uStripesPower;
uniform float uVinettePower;
uniform float uGrainPower;

// based on aberation by hornet
// https://www.shadertoy.com/view/XssGz8
// To the extent possible under law,
// the author has waived all copyright and related or neighboring rights to this work.

#define EPSILON 1e-10

const float gamma = 2.2;

float saturate(float v) { return clamp(v, 0.0, 1.0); }
vec2  saturate(vec2  v) { return clamp(v, vec2(0.0), vec2(1.0)); }
vec3  saturate(vec3  v) { return clamp(v, vec3(0.0), vec3(1.0)); }
vec4  saturate(vec4  v) { return clamp(v, vec4(0.0), vec4(1.0)); }

vec3 ColorTemperatureToRGB(float temperatureInKelvins)
{
	vec3 retColor;

	temperatureInKelvins = clamp(temperatureInKelvins, 1000.0, 40000.0) / 100.0;

	if (temperatureInKelvins <= 66.0)
	{
		retColor.r = 1.0;
		retColor.g = saturate(0.39008157876901960784 * log(temperatureInKelvins) - 0.63184144378862745098);
	}
	else
	{
		float t = temperatureInKelvins - 60.0;
		retColor.r = saturate(1.29293618606274509804 * pow(t, -0.1332047592));
		retColor.g = saturate(1.12989086089529411765 * pow(t, -0.0755148492));
	}

	if (temperatureInKelvins >= 66.0)
		retColor.b = 1.0;
	else if (temperatureInKelvins <= 19.0)
		retColor.b = 0.0;
	else
		retColor.b = saturate(0.54320678911019607843 * log(temperatureInKelvins - 10.0) - 1.19625408914);

	return retColor;
}

float Luminance(vec3 color)
{
	float fmin = min(min(color.r, color.g), color.b);
	float fmax = max(max(color.r, color.g), color.b);
	return (fmax + fmin) / 2.0;
}

vec3 HUEtoRGB(float H)
{
	float R = abs(H * 6.0 - 3.0) - 1.0;
	float G = 2.0 - abs(H * 6.0 - 2.0);
	float B = 2.0 - abs(H * 6.0 - 4.0);
	return saturate(vec3(R, G, B));
}

vec3 HSLtoRGB(in vec3 HSL)
{
	vec3 RGB = HUEtoRGB(HSL.x);
	float C = (1.0 - abs(2.0 * HSL.z - 1.0)) * HSL.y;
	return (RGB - 0.5) * C + vec3(HSL.z);
}

vec3 RGBtoHCV(vec3 RGB)
{
	// Based on work by Sam Hocevar and Emil Persson
	vec4 P = (RGB.g < RGB.b) ? vec4(RGB.bg, -1.0, 2.0 / 3.0) : vec4(RGB.gb, 0.0, -1.0 / 3.0);
	vec4 Q = (RGB.r < P.x) ? vec4(P.xyw, RGB.r) : vec4(RGB.r, P.yzx);
	float C = Q.x - min(Q.w, Q.y);
	float H = abs((Q.w - Q.y) / (6.0 * C + EPSILON) + Q.z);
	return vec3(H, C, Q.x);
}

vec3 RGBtoHSL(vec3 RGB)
{
	vec3 HCV = RGBtoHCV(RGB);
	float L = HCV.z - HCV.y * 0.5;
	float S = HCV.y / (1.0 - abs(L * 2.0 - 1.0) + EPSILON);
	return vec3(HCV.x, S, L);
}

float remap(float t, float a, float b) {
	return clamp((t - a) / (b - a), 0.0, 1.0);
}

vec2 remap(vec2 t, vec2 a, vec2 b) {
	return clamp((t - a) / (b - a), 0.0, 1.0);
}

//note: input [0;1]
vec3 spectrum_offset_rgb(float t)
{
	//note: optimisation from https://twitter.com/Stubbesaurus/status/818847844790575104
	//t = 3.0 * t - 0.5;
	//vec3 ret = clamp( vec3(1.0-t, 1.0-abs(t-1.0), t-1.0), 0.0, 1.0);
	float t0 = 3.0 * t - 1.5;
	vec3 ret = clamp(vec3(-t0, 1.0 - abs(t0), t0), 0.0, 1.0);

	return ret;
	//return smoothstep( vec3(0.0), vec3(1.0), ret );
	//return pow( ret, vec3(1.0/2.2) );

}

vec3 lin2srgb(vec3 c)
{
	return pow(c, vec3(gamma));
}

vec3 srgb2lin(vec3 c)
{
	return pow(c, vec3(1.0 / gamma));
}

vec3 yCgCo2rgb(vec3 ycc)
{
	float R = ycc.x - ycc.y + ycc.z;
	float G = ycc.x + ycc.y;
	float B = ycc.x - ycc.y - ycc.z;
	return vec3(R, G, B);
}

vec3 spectrum_offset_ycgco(float t)
{
	//vec3 ygo = vec3( 1.0, 1.5*t, 0.0 ); //green-pink
	//vec3 ygo = vec3( 1.0, -1.5*t, 0.0 ); //green-purple
	vec3 ygo = vec3(1.0, 0.0, -1.25*t); //cyan-orange
										//vec3 ygo = vec3( 1.0, 0.0, 1.5*t ); //brownyello-blue
	return yCgCo2rgb(ygo);
}

vec3 yuv2rgb(vec3 yuv)
{
	vec3 rgb;
	rgb.r = yuv.x + yuv.z * 1.13983;
	rgb.g = yuv.x + dot(vec2(-0.39465, -0.58060), yuv.yz);
	rgb.b = yuv.x + yuv.y * 2.03211;
	return rgb;
}

//note: from https://www.shadertoy.com/view/XslGz8
vec2 radialdistort(vec2 coord, vec2 amt)
{
	vec2 cc = coord - 0.5;
	return coord + 2.0 * cc * amt;
}

// Given a vec2 in [-1,+1], generate a texture coord in [0,+1]
vec2 barrelDistortion(vec2 p, vec2 amt)
{
	p = 2.0 * p - 1.0;

	/*
	const float maxBarrelPower = 5.0;
	//note: http://glsl.heroku.com/e#3290.7 , copied from Little Grasshopper
	float theta  = atan(p.y, p.x);
	vec2 radius = vec2( length(p) );
	radius = pow(radius, 1.0 + maxBarrelPower * amt);
	p.x = radius.x * cos(theta);
	p.y = radius.y * sin(theta);

	/*/
	// much faster version
	//const float maxBarrelPower = 5.0;
	//float radius = length(p);
	float maxBarrelPower = sqrt(5.0);
	float radius = dot(p, p); //faster but doesn't match above accurately
	p *= pow(vec2(radius), maxBarrelPower * amt);
	/* */

	return p * 0.5 + 0.5;
}

//note: from https://www.shadertoy.com/view/MlSXR3
vec2 brownConradyDistortion(vec2 uv, float dist)
{
	uv = uv * 2.0 - 1.0;
	// positive values of K1 give barrel distortion, negative give pincushion
	float barrelDistortion1 = 0.1 * dist; // K1 in text books
	float barrelDistortion2 = -0.025 * dist; // K2 in text books

	float r2 = dot(uv, uv);
	uv *= 1.0 + barrelDistortion1 * r2 + barrelDistortion2 * r2 * r2;
	//uv *= 1.0 + barrelDistortion1 * r2;

	// tangential distortion (due to off center lens elements)
	// is not modeled in this function, but if it was, the terms would go here
	return uv * 0.5 + 0.5;
}

vec2 distort(vec2 uv, float t, vec2 min_distort, vec2 max_distort)
{
	vec2 dist = mix(min_distort, max_distort, t);
	// return radialdistort( uv, 2.0 * dist );
	// return barrelDistortion( uv, 1.75 * dist ); //distortion at center
	return brownConradyDistortion(uv, 75.0 * dist.x);
}

vec3 spectrum_offset_yuv(float t)
{
	//vec3 yuv = vec3( 1.0, 3.0*t, 0.0 ); //purple-green
	//vec3 yuv = vec3( 1.0, 0.0, 2.0*t ); //purple-green
	vec3 yuv = vec3(1.0, 0.0, -1.0*t); //cyan-orange
									   //vec3 yuv = vec3( 1.0, -0.75*t, 0.0 ); //brownyello-blue
	return yuv2rgb(yuv);
}

vec3 spectrum_offset(float t)
{
	return spectrum_offset_rgb(t);
	//return srgb2lin( spectrum_offset_rgb( t ) );
	//return lin2srgb( spectrum_offset_rgb( t ) );

	//return spectrum_offset_ycgco( t );
	//return spectrum_offset_yuv( t );
}

float nrand(vec2 n)
{
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

vec3 render(vec2 uv)
{
	return srgb2lin(texture(i_color, uv).rgb);
}

vec3 Tonemap_ACES(const vec3 x) {
	// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return (x * (a * x + b)) / (x * (c * x + d) + e);
}

float stripes(vec2 uv)
{
	return mix(0.9, 1.0, floor(0.5 + fract(uv.y*0.25*uResolution.y)));
}

float grain(vec2 uv)
{
	float strength = 16.0;
	float x = (uv.x + 4.0) * (uv.y + 4.0) * (uTime * 10.0);
	float grain = (mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005) * strength;
	return 1.0 - grain;
}

float vinette(vec2 uv)
{
	float dist = length((vTexCoord - vec2(0.5, 0.5)) * 1.43f);
	float vinette = 1.0 - dist * dist;
	return mix(0.2, 1.0, vinette);
}

vec3 colorTemperature(vec3 color, float temperature)
{
	vec3 colorTempRGB = ColorTemperatureToRGB(temperature);
	float originalLuminance = Luminance(color);
	vec3 blended = mix(color, color * colorTempRGB, uColorTempPower);
	vec3 resultHSL = RGBtoHSL(blended);
	vec3 luminancePreservedRGB = HSLtoRGB(vec3(resultHSL.x, resultHSL.y, originalLuminance));
	return mix(blended, luminancePreservedRGB, uColorTempLuminancePreservation);
}

vec3 Saturation(vec3 color, float saturation)
{
	vec3 hsl = RGBtoHSL(color);
	hsl.g *= saturation;
	return HSLtoRGB(hsl);
}

vec3 filmGrainColor(vec2 uv, float offset)
{ // by ma (lstGWn)
    vec4 uvs;
    uvs.xy = uv + vec2(offset, offset);
    uvs.zw = uvs.xy + 0.5 * vec2(1.0 / uResolution.x, 1.0 / uResolution.y);

    uvs = fract(uvs * vec2(21.5932, 21.77156).xyxy);

    vec2 shift = vec2(21.5351, 14.3137);
    vec2 temp0 = uvs.xy + dot(uvs.yx, uvs.xy + shift);
    vec2 temp1 = uvs.xw + dot(uvs.wx, uvs.xw + shift);
    vec2 temp2 = uvs.zy + dot(uvs.yz, uvs.zy + shift);
    vec2 temp3 = uvs.zw + dot(uvs.wz, uvs.zw + shift);

    vec3 r = vec3(0.0, 0.0, 0.0);
    r += fract(temp0.x * temp0.y * vec3(95.4337, 96.4337, 97.4337));
    r += fract(temp1.x * temp1.y * vec3(95.4337, 96.4337, 97.4337));
    r += fract(temp2.x * temp2.y * vec3(95.4337, 96.4337, 97.4337));
    r += fract(temp3.x * temp3.y * vec3(95.4337, 96.4337, 97.4337));

    return r * 0.25;
}

vec3 sharpen(vec2 uv)
{ // by NickWest (lslGRr)
    vec2 step = 1.0 / uResolution.xy;
    float scale = 1.5;
    vec3 texA = texture(i_color, uv + vec2(-step.x, -step.y) * scale).rgb;
    vec3 texB = texture(i_color, uv + vec2(step.x, -step.y) * scale).rgb;
    vec3 texC = texture(i_color, uv + vec2(-step.x, step.y) * scale).rgb;
    vec3 texD = texture(i_color, uv + vec2(step.x, step.y) * scale).rgb;
    vec3 around = 0.25 * (texA + texB + texC + texD);
    vec3 center = texture(i_color, uv).rgb;
    vec3 col = center + (center - around) * 0.2;
    return col;
}

vec2 barrelDistortion(vec2 coord, float amt, float zoom)
{ // based on gtoledo3 (XslGz8)
  // added zoomimg
    vec2 cc = coord - 0.5;
    vec2 p = cc * zoom;
    coord = p + 0.5;
    float dist = dot(cc, cc);
    return coord + cc * dist * amt;
    
}

void main()
{
	vec2 uv = vTexCoord;
    uv = barrelDistortion(uv, 0.1, 0.94);
    
    color.rgb = sharpen(uv);

    // color.rgb = 0.5 * pow(color.rgb, vec3(1.2));

	// color saturation
	// color.rgb = Saturation(color.rgb, 0.9);

	// tonemapper
	// color.rgb = Tonemap_ACES(color.rgb);
	
	// color temperature
	// color.rgb = colorTemperature(color.rgb, mix(6500.0, uColorTempValue, uColorTempPower));
	
	
	// grain
    // float FGanim = 0.01 * smoothstep(-0.8, 0.8, sin(5.0 * uTime));
    color.rgb *= 1.0 - 0.1 * filmGrainColor(0.01 * uv, uTime).rgb;

	// Vinette
    // color.rgb = vec3(1.0); // uncomment to see only vignette
    vec2 v = 2. * (uv - .5);
    v = clamp((v * .5) + .5, 0., 1.);
    color.rgb *= 0.25 + pow(16.0 * v.x * v.y * (1.0 - v.x) * (1.0 - v.y), 0.25);
	
	// gamma 
	color.rgb = pow(color.rgb, vec3(0.4545));
}
