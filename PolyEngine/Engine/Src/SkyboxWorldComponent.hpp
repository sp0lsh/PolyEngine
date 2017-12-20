#pragma once

#include "ComponentBase.hpp"
#include "CubemapResource.hpp"
#include "TextureResource.hpp"

namespace Poly
{
	enum class eSkyboxType
	{
		CUBEMAP,
		PANORAMA,
		_COUNT
	};

	class ENGINE_DLLEXPORT SkyboxWorldComponent : public ComponentBase
	{
	public:
		SkyboxWorldComponent(const Dynarray<String> cubemapPath);
		// SkyboxWorldComponent(const String panoramaPath);
		const eSkyboxType GetSkyboxType() const { return Type; }
		const CubemapResource& GetCubemap() const { return Cubemap; }
		// const TextureResource& GetPanorama() const { return Panorama; }

	private:
		eSkyboxType Type;
		CubemapResource Cubemap;
		// TextureResource Panorama;
	};

	REGISTER_COMPONENT(WorldComponentsIDGroup, SkyboxWorldComponent)
}