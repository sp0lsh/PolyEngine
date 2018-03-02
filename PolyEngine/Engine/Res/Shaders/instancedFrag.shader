#version 330 core

uniform sampler2D i_color;
uniform float uTime;
uniform float uSpeed;
uniform vec4 uColor;
uniform float uHasSprite;
// uniform vec2 uSubImages;

in vec2 vTexCoord;
in float vInstanceID;
out vec4 color;

float nrand(float n)
{
    return fract(sin(dot(n, 12.9898)) * 43758.5453);
}

vec2 SubUV(vec2 uv, vec2 subImages, float frame)
{
    vec2 resRcp = vec2(1.0) / subImages;
    float frameInt = frame - fract(frame);
    float uvTileX = mod(frameInt, subImages.x);
    float uvTileY = floor(frameInt * resRcp.x);
    
    vec2 uvTile = (vec2(uvTileX, uvTileY) + uv) * resRcp;
    return uvTile;
}

void main()
{
    vec2 c = 2.0 * (vTexCoord - 0.5);
    float mask = 0.5 * (1.0 - clamp(dot(c, c), 0.0, 1.0));

    vec2 uv = vTexCoord.rg;
    vec2 uSubImages = vec2(2.0, 2.0);
    // float uSpeedTime = 1.0;
    float InstanceRnd = nrand(vInstanceID);
    // float uFrame = InstanceRnd * uSubImages.x * uSubImages.y;
	float uFrame = 0.0;
//     float uSpeed = 1.0;
    float uSpeedPow = 1.0;

    float frame = uFrame + uSubImages.x * uSubImages.y * fract(-1.0 * uSpeed * uTime);

    vec2 uvTile0 = SubUV(uv, uSubImages, frame);
    vec2 uvTile1 = SubUV(uv, uSubImages, frame + 1);

    vec4 tex0 = texture(i_color, uvTile0);
    vec4 tex1 = texture(i_color, uvTile1);
    vec4 tex = mix(tex0, tex1, fract(frame));

    color = mix(vec4(mask), tex, uHasSprite);
    color *= uColor;
}
