#version 330 core

uniform sampler2D i_color;
uniform sampler2D i_noise;
uniform float uTime;
uniform vec2 uResolution;
uniform vec4 uCameraPosition;
uniform mat4 uCameraRotation;

uniform vec4 uShipPos;
uniform vec4 uShipFwd;

in vec2 vTexCoord;
out vec4 o_color;

// #define CASHETES

/*
* "Seascape" by Alexander Alekseev aka TDM - 2014
* License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
* Contact: tdmaav@gmail.com
*/

const int NUM_STEPS = 8; // 8
const float PI = 3.141592;
const float EPSILON = 1e-3;
#define EPSILON_NRM (0.1 / uResolution.x)

// sea
const int ITER_GEOMETRY = 3; // 3
const int ITER_FRAGMENT = 5; // 5
const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.16;
const vec3 SEA_BASE = vec3(0.1, 0.19, 0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8, 0.9, 0.6);
#define SEA_TIME (1.0 + uTime * SEA_SPEED)
const mat2 octave_m = mat2(1.6, 1.2, -1.2, 1.6);

const vec3 SUN_POWER = vec3(1.0, 0.9, 0.6) * 750.0;
const vec3 albedo = 0.1 * vec3(0.95, 0.16, 0.015);
const vec3 LIGHT_DIR = normalize(vec3(0.0, 1.0, 0.8));

// math
mat3 fromEuler(vec3 ang) {
	vec2 a1 = vec2(sin(ang.x), cos(ang.x));
	vec2 a2 = vec2(sin(ang.y), cos(ang.y));
	vec2 a3 = vec2(sin(ang.z), cos(ang.z));
	mat3 m;
	m[0] = vec3(a1.y*a3.y + a1.x*a2.x*a3.x, a1.y*a2.x*a3.x + a3.y*a1.x, -a2.y*a3.x);
	m[1] = vec3(-a2.y*a1.x, a1.y*a2.y, a2.x);
	m[2] = vec3(a3.y*a1.x*a2.x + a1.y*a3.x, a1.x*a3.x - a1.y*a3.y*a2.x, a2.y*a3.y);
	return m;
}
float hash(vec2 p) {
	float h = dot(p, vec2(127.1, 311.7));
	return fract(sin(h)*43758.5453123);
}
float noise(in vec2 p) {
	vec2 i = floor(p);
	vec2 f = fract(p);
	vec2 u = f*f*(3.0 - 2.0*f);
	return -1.0 + 2.0*mix(mix(hash(i + vec2(0.0, 0.0)),
		hash(i + vec2(1.0, 0.0)), u.x),
		mix(hash(i + vec2(0.0, 1.0)),
			hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

// lighting
float diffuse(vec3 n, vec3 l, float p) {
	return pow(dot(n, l) * 0.4 + 0.6, p);
}
float specular(vec3 n, vec3 l, vec3 e, float s) {
	float nrm = (s + 8.0) / (PI * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

// sky
vec3 getSkyColor(vec3 e) {
	e.y = max(e.y, 0.0);
	return vec3(pow(1.0 - e.y, 2.0), 1.0 - e.y, 0.6 + (1.0 - e.y)*0.4);
}

// sea
float sea_octave(vec2 uv, float choppy) {
	uv += noise(uv);
	vec2 wv = 1.0 - abs(sin(uv));
	vec2 swv = abs(cos(uv));
	wv = mix(wv, swv, wv);
	return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy);
}

float map(vec3 p) {
	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2 uv = p.xz; uv.x *= 0.75;

	float d, h = 0.0;
	for (int i = 0; i < ITER_GEOMETRY; i++) {
		d = sea_octave((uv + SEA_TIME)*freq, choppy);
		d += sea_octave((uv - SEA_TIME)*freq, choppy);
		h += d * amp;
		uv *= octave_m; freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.y - h;
}

float map_detailed(vec3 p) {
	float freq = SEA_FREQ;
	float amp = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2 uv = p.xz; uv.x *= 0.75;

	float d, h = 0.0;
	for (int i = 0; i < ITER_FRAGMENT; i++) {
		d = sea_octave((uv + SEA_TIME)*freq, choppy);
		d += sea_octave((uv - SEA_TIME)*freq, choppy);
		h += d * amp;
		uv *= octave_m; freq *= 1.9; amp *= 0.22;
		choppy = mix(choppy, 1.0, 0.2);
	}
	return p.y - h;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {
	float fresnel = clamp(1.0 - dot(n, -eye), 0.0, 1.0);
	fresnel = pow(fresnel, 3.0) * 0.65;

	vec3 reflected = getSkyColor(reflect(eye, n));
	vec3 refracted = SEA_BASE + diffuse(n, l, 80.0) * SEA_WATER_COLOR * 0.12;

	vec3 color = mix(refracted, reflected, fresnel);

	float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0);
	color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;

	color += vec3(specular(n, l, eye, 60.0));

	return color;
}

// tracing
vec3 getNormal(vec3 p, float eps) {
	vec3 n;
	n.y = map_detailed(p);
	n.x = map_detailed(vec3(p.x + eps, p.y, p.z)) - n.y;
	n.z = map_detailed(vec3(p.x, p.y, p.z + eps)) - n.y;
	n.y = eps;
	return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {
	float tm = 0.0;
	float tx = 1000.0;
	float hx = map(ori + dir * tx);
	if (hx > 0.0) return tx;
	float hm = map(ori + dir * tm);
	float tmid = 0.0;
	for (int i = 0; i < NUM_STEPS; i++) {
		tmid = mix(tm, tx, hm / (hm - hx));
		p = ori + dir * tmid;
		float hmid = map(p);
		if (hmid < 0.0) {
			tx = tmid;
			hx = hmid;
		}
		else {
			tm = tmid;
			hm = hmid;
		}
	}
	return tmid;
}

vec4 renderWater(vec3 ro, vec3 rd)
{
    vec3 p;
    heightMapTracing(ro, rd, p);
    vec3 dist = p - ro;
    vec3 n = getNormal(p, dot(dist, dist) * EPSILON_NRM);
    vec3 light = LIGHT_DIR;
             
    // color
    vec3 color = mix(
        getSkyColor(rd),
        getSeaColor(p, n, light, rd, dist),
    	pow(smoothstep(0.0, -0.05, rd.y), 0.3));
    
    float depth = dot(rd, dist) * length(dist);
    // float depth = length(dist);
    return vec4(color, depth);
}

float sdCylinder(vec3 p, vec2 h)
{
    vec2 d = abs(vec2(length(p.xz), p.y)) - h;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float sdBox(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

// polynomial smooth min (k = 0.1);
float smin(float a, float b, float k)
{
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}

float sdShip(vec3 p)
{
    float s2 = sdCylinder(p - vec3(0.85, 2.0, 0.0), vec2(0.35, 1.0));
    float s7 = sdCylinder(p - vec3(0.85, 2.2, 0.0), vec2(0.3, 1.0));
    float s4 = sdBox(p - vec3(-0.2, 1.7, 0.0), vec3(0.6, 0.4, 0.8));
    float s5 = sdBox(p - vec3(1.3, 1.6, 0.0), vec3(1.0, 0.2, 0.6));
    float s6 = sdBox(p - vec3(-0.3, 2.1, 0.0), vec3(0.7, 0.05, 0.82));
    
    p.xz *= 1.0 - 0.25 * (p.y - 0.3);
    p.y -= 0.05 * (p.x * p.x) * max(-p.x, 0.0);
    
    vec3 p0 = p;
    p0.z = abs(p0.z);
    p0.z = p0.z + 0.5;
    float s1 = sdBox(p - vec3(1.0, 1.0, 0.0), vec3(1.0, 0.5, 0.85));
    p0.x *= 0.7;
    float s3 = sdCylinder(p0 - vec3(0.0, 1.0, 0.0), vec2(1.35, 0.5));
    
    float s = smin(s1, s3, 0.1);
    s = min(s, s2);
    s = min(s, s3);
    s = min(s, s4);
    s = min(s, s5);
    s = min(s, s6);
    s = max(-s7, s);
    s -= 0.02;
    return s;
}

mat3 lookAt(in vec3 ro, in vec3 ta, float cr)
{
    vec3 cw = normalize(ta - ro);
    vec3 cp = vec3(sin(cr), cos(cr), 0.0);
    vec3 cu = normalize(cross(cw, cp));
    vec3 cv = normalize(cross(cu, cw));
    return mat3(cu, cv, cw);
}

mat2 rot2D(float a)
{
    float s = sin(a);
    float c = cos(a);
    return mat2(c, s, -s, c);
}

float mapShips(vec3 p)
{
    vec3 ps = p - uShipPos.xyz;
    ps.xz *= rot2D(-uShipPos.w);
    return sdShip(ps);
}

vec3 getNormalShips(vec3 p)
{
    vec2 e = vec2(1.0, -1.0) * 0.5773 * 0.0005;
    return normalize(e.xyy * mapShips(p + e.xyy) +
					  e.yyx * mapShips(p + e.yyx) +
					  e.yxy * mapShips(p + e.yxy) +
				  	  e.xxx * mapShips(p + e.xxx));
}

float calcAO(in vec3 pos, in vec3 nor)
{
    float sca = 10.2;
    float hr = 0.05;
    float dd = mapShips(nor * 0.15 + pos);
    return clamp(1.0 + (dd - hr) * sca, 0.0, 1.0);
}

float softshadow(in vec3 ro, in vec3 rd, in float mint, in float tmax)
{
    float res = 1.0;
    float t = mint;
    for (int i = 0; i < 14; i++)
    {
        float h = mapShips(ro + rd * t);
        res = min(res, 8. * h / t);
        t += clamp(h, 0.08, 0.25);
        if (res < 0.001 || t > tmax)
            break;
    }
    return max(0.0, res);
}

float Schlick(float f0, float VoH)
{
    return f0 + (1. - f0) * pow(1.0 - VoH, 5.0);
}

vec3 renderCube(vec3 pos, vec3 sun_direction, vec3 dir)
{
    vec3 nor = getNormalShips(pos);
    float occ = calcAO(pos, nor);
    float NoL = max(0.0, dot(sun_direction, nor));
    float sunShadow = softshadow(pos, sun_direction, 0.001, 2.0);
    vec3 color = 0.6 * NoL * SUN_POWER * albedo * sunShadow / PI; // diffuse
    color += albedo * occ * vec3(0.3, 0.6, 1.0) * 35.0 * (0.75 + 0.25 * nor.y); // skylight
    color += Schlick(0.04, max(0.0, dot(nor, -dir))) * max(0.0, occ - 0.7) / 0.7; // specular
    return color;
}

vec4 renderShips(vec3 ro, vec3 rd)
{
    vec3 color = vec3(0.0);
    vec3 p = vec3(0.0);
    float t = 0.0;
    float d = 0.0;
    for (int i = 0; i < 200; ++i)
    {
        t += (d = mapShips(p = ro + rd * t));
        if (t < 0.01)
        {
            break;
        }
    }
        
    color = renderCube(p, LIGHT_DIR, rd);
    
    vec3 pos = ro + rd * t;
    vec3 dist = pos - ro;
    float depth = dot(rd, dist) * length(dist);
    return vec4(color, depth);
}

void main()
{

#ifdef CASHETES
	vec2 cp = -1.0 + 2.0 * vTexCoord.xy;
	cp.x *= uResolution.x / uResolution.y;
	float cashetes = step(abs(cp.y)*2.39, uResolution.x / uResolution.y);
	if (cashetes<0.1) {
		o_color = vec4(0.0);
		return;
	}
#endif

	vec2 uv = vTexCoord.xy;
	uv = uv * 2.0 - 1.0;
	uv.x *= uResolution.x / uResolution.y;
	float time = uTime * 0.3;

	vec3 ro = uCameraPosition.xyz;
	vec3 rd = normalize(vec3(uv.xy, -2.0));
	rd.z += length(uv) * 0.15;
	rd = (uCameraRotation * vec4(rd, 1.0)).xyz;
	rd = normalize(rd);

	// tracing
    vec4 water = renderWater(ro, rd);
    // water.rgb *= fract(1.0 *water.a); // debug depth
    
    vec4 ships = renderShips(ro, rd);
    // ships.rgb *= fract(1.0 * ships.a); // debug depth

    // float depthFade = clamp(max(0.0, water.a - ships.a), 0.0, 1.0);
    vec3 color = ships.a < water.a ? ships.rgb : water.rgb;
    
    // post
    color = mix(color, smoothstep(color, vec3(0.0), vec3(1.0)), 0.5);
	// color = pow(color, vec3(0.45));

	// post
    o_color = vec4(color, 1.0);
}
