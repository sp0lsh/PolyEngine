#pragma once

#include "ComponentBase.hpp"
#include "RenderingSystem.hpp"
#include "ParticleResource.hpp"

namespace Poly {

	class ENGINE_DLLEXPORT ParticleComponent : public ComponentBase
	{
		friend void RenderingSystem::RenderingPhase(World*);
	public:
		ParticleComponent(int count);
		virtual ~ParticleComponent();

		int GetCount() const { return Count; }
		void SetCount(int value) { Count = value; }

	private:
		int Count = 0;
		ParticleResource Resource;

	};

	REGISTER_COMPONENT(ComponentsIDGroup, ParticleComponent)
}