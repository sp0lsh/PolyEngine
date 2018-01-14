#pragma once

// #include <Dynarray.hpp>
// #include <EnumUtils.hpp>
// #include <Color.hpp>

#include "ResourceBase.hpp"
// #include "TextureResource.hpp"
// #include "Mesh.hpp"

namespace Poly
{
	class ENGINE_DLLEXPORT ParticleResource : public ResourceBase
	{
	public:
		ParticleResource();
		virtual ~ParticleResource();
		
		const IParticleDeviceProxy* GetParticleProxy() const { return ParticleProxy.get(); }

	private:
		Particle ParticleData;
		std::unique_ptr<IParticleDeviceProxy> ParticleProxy;
	};
}