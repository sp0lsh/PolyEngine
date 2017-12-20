#include "EnginePCH.hpp"

#include "SkyboxWorldComponent.hpp"

Poly::SkyboxWorldComponent::SkyboxWorldComponent(const Dynarray<String> cubemapPath)
	: Type(eSkyboxType::CUBEMAP), Cubemap(cubemapPath) /*, Panorama("") */
{
	gConsole.LogInfo("SkyboxWorldComponent::SkyboxWorldComponent cubemapPath: {}", cubemapPath[0]);
}

// Poly::SkyboxWorldComponent::SkyboxWorldComponent(const String panoramaPath)
// 	: Type(eSkyboxType::PANORAMA), Cubemap({}), Panorama(panoramaPath)
// {
// 	gConsole.LogInfo("SkyboxWorldComponent::SkyboxWorldComponent panoramaPath: {}", panoramaPath);
// }
