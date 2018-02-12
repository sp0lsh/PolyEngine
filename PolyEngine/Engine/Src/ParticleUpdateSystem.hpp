#pragma once

namespace Poly
{
	class World;
	class EntityTransform;

	namespace ParticleUpdateSystem
	{
		void ParticleUpdatePhase(World* world);
		void EmitterEmit(World*, ParticleEmitter*);
		void EmitterUpdate(World*, ParticleEmitter*);
	}
}