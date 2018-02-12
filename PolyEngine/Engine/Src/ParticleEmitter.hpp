#pragma once

#include <BaseObject.hpp>
#include <IRenderingDevice.hpp>
#include <Dynarray.hpp>
#include <SpritesheetComponent.hpp>
#include "ParticleUpdateSystem.hpp"

typedef unsigned int GLuint;

namespace Poly
{

	class ENGINE_DLLEXPORT ParticleEmitter : public BaseObject<>
	{
		friend void ParticleUpdateSystem::EmitterEmit(World*, ParticleEmitter*);
		friend void ParticleUpdateSystem::EmitterUpdate(World*, ParticleEmitter*);
		friend void ParticleUpdateSystem::EmitterRecreateBuffer(World*, ParticleEmitter*);
	public:

		enum class eSpawnSpace
		{
			LOCAL_SPACE,
			WORLD_SPACE,
			_COUNT
		};

		struct ENGINE_DLLEXPORT Particle
		{
			Vector Position;
			Vector Scale;
			Vector Velocity;
			Vector Acceleration;
			float Age;
			float LifeTime;
		};

		struct ENGINE_DLLEXPORT Settings
		{
			int MaxSize = 1000;
			int InitialSize = 10;
			float BurstTimeMin = 1.0f;
			float BurstTimeMax = 2.0f;
			int BurstSizeMin = 1;
			int BurstSizeMax = 5;
			eSpawnSpace SpawnSpace = eSpawnSpace::WORLD_SPACE;
			// TODO: move to material
			SpritesheetSettings SpritesheetSettings;
			float Speed = 1.0f;
			Color Color = Color::WHITE;
			// TODO end
			std::function<void(Particle*)> ParticleInitFunc;
			std::function<void(Particle*)> ParticleUpdateFunc;
		};

		ParticleEmitter(const Settings& settings);
		~ParticleEmitter();

		const Dynarray<float>& GetInstances() const { return InstancesTransform; }
		const IParticleDeviceProxy* GetParticleProxy() const { return ParticleProxy.get(); }

		void Emit(size_t quota);

		bool GetIsBurstEnabled() { return IsBurstEnabled; }
		
		const Settings& GetSettings() const { return settings; }
		const TextureResource* GetSpritesheet() const { return Spritesheet; }

		void SetBurstEnabled(bool value) { IsBurstEnabled = value; }

		// void Update(World* world);

		// void RecreateBufferForProxy();

		bool HasInstances() const { return InstancesTransform.GetSize() != 0; }

	private:
		Settings settings;
		TextureResource* Spritesheet;

		IterablePoolAllocator<Particle> ParticlesPool;

		Dynarray<float> InstancesTransform;
		std::unique_ptr<IParticleDeviceProxy> ParticleProxy;

		void UpdateDeviceProxy();

		float NextBurstTime = -1.0;

		bool IsBurstEnabled = true;

		int toEmit = 0;
	};
}