#include "EnginePCH.hpp"

#include "SkyboxWorldComponent.hpp"

Poly::SkyboxWorldComponent::SkyboxWorldComponent(const EnumArray<String, eCubemapSide> cubemapPath)
	: Cubemap(cubemapPath)
{
	gConsole.LogInfo("SkyboxWorldComponent::SkyboxWorldComponent cubemapPath: {}", cubemapPath[eCubemapSide::LEFT]);
}

// Poly::SkyboxWorldComponent::SkyboxWorldComponent(const String panoramaPath)
// 	: Type(eSkyboxType::PANORAMA), Cubemap({}), Panorama(panoramaPath)
// {
// 	gConsole.LogInfo("SkyboxWorldComponent::SkyboxWorldComponent panoramaPath: {}", panoramaPath);
// }
