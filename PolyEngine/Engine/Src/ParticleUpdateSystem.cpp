#include "EnginePCH.hpp"

#include "ParticleUpdateSystem.hpp"
#include "ParticleComponent.hpp"

using namespace Poly;

void ParticleUpdateSystem::ParticleUpdatePhase(World* world)
{
	// gConsole.LogInfo("ParticleUpdateSystem::ParticleUpdatePhasee");

	float deltaTime = (float)(TimeSystem::GetTimerDeltaTime(world, Poly::eEngineTimer::GAMEPLAY));

	// Render meshes
	for (auto compTuple : world->IterateComponents<ParticleComponent>())
	{
		ParticleComponent* particle = std::get<ParticleComponent*>(compTuple);
		// particle->GetEmitter()->Update(world);
		EmitterEmit(world, particle->Emitter);
		EmitterUpdate(world, particle->Emitter);
	}
}

void ParticleUpdateSystem::EmitterEmit(World* world, ParticleEmitter* emitter)
{
	size_t size = emitter->toEmit;
	emitter->toEmit = 0;
	size_t sizeLeft = emitter->ParticlesPool.GetFreeBlockCount();
	if (size > sizeLeft)
	{
		gConsole.LogInfo("ParticleEmitter::Emit not enough memory in room (1000)");
	}

	size_t amount = Clamp(size, (size_t)0, sizeLeft);

	// gConsole.LogInfo("ParticleEmitter::Emit emitLen: {}", amount);

	while (amount > 0)
	{
		ParticleEmitter::Particle* p = emitter->ParticlesPool.Alloc();
		::new(p) ParticleEmitter::Particle();
		
		emitter->settings.ParticleInitFunc(p);

		--amount;
	}
}

void ParticleUpdateSystem::EmitterUpdate(World* world, ParticleEmitter* emitter)
{
	gConsole.LogInfo("ParticleEmitter::Update {}/{}", emitter->ParticlesPool.GetSize(), emitter->settings.MaxSize);

	float deltaTime = (float)(TimeSystem::GetTimerDeltaTime(world, Poly::eEngineTimer::GAMEPLAY));

	if (emitter->IsBurstEnabled)
	{
		if (emitter->NextBurstTime < 0.0f)
		{
			emitter->NextBurstTime = RandomRange(emitter->settings.BurstTimeMin, emitter->settings.BurstTimeMax);
			emitter->Emit((int)RandomRange(emitter->settings.BurstSizeMin, emitter->settings.BurstSizeMax));
		}
		else
		{
			emitter->NextBurstTime -= deltaTime;
		}
	}

	Dynarray<ParticleEmitter::Particle*> ParticleToDelete;

	for (ParticleEmitter::Particle& p : emitter->ParticlesPool)
	{
		p.Age += deltaTime;
		if (p.Age > p.LifeTime)
		{
			ParticleToDelete.PushBack(&p);
		}
	}

	// if (ParticleToDelete.GetSize() > 0)
	// {
	// 	gConsole.LogInfo("ParticleEmitter::Update toDeleteLen: {}", ParticleToDelete.GetSize());
	// }

	for (ParticleEmitter::Particle* p : ParticleToDelete)
	{
		p->~Particle();
		emitter->ParticlesPool.Free(p);
	}

	for (ParticleEmitter::Particle& p : emitter->ParticlesPool)
	{
		emitter->settings.ParticleUpdateFunc(&p);
	}

	emitter->RecreateBufferForProxy();
	emitter->UpdateDeviceProxy();
}
