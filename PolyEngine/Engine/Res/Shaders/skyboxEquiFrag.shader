#version 330 core

in vec3 vUV;

out vec4 color;

uniform samplerCube uCubemap;

// https://github.com/Unity-Technologies/SkyboxPanoramicShader/blob/master/Skybox-PanoramicBeta.shader
vec2 ToRadialCoords(vec3 coords)
{
    vec3 normalizedCoords = normalize(coords);
    float latitude = acos(normalizedCoords.y);
    float longitude = atan(normalizedCoords.x, normalizedCoords.z);
    vec2 sphereCoords = vec2(longitude, latitude) * vec2(0.5 / 3.14, 1.0 / 3.14);
    return vec2(0.5, 1.0) - sphereCoords;
}

void main()
{
    // vec2 longlat = vec2(atan2(dir.x, dir.z) + UNITY_PI, acos(-dir.y));
    vec2 longlat = ToRadialCoords(vUV);
	// color = vec4(texture(uCubemap, vUV).rgb, 1.0);
    color = vec4(longlat, 0.0, 1.0);

}