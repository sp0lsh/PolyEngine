#include "EnginePCH.hpp"
#include "ParticleEmitter.hpp"

#include <algorithm>

using namespace Poly;

ParticleEmitter::ParticleEmitter(const Settings& settings)
	: settings(settings), ParticlesPool(settings.MaxSize), toEmit(settings.InitialSize)
{
	Spritesheet = ResourceManager<TextureResource>::Load(
		settings.SpritesheetSettings.SpritePath,
		settings.SpritesheetSettings.Source,
		eTextureUsageType::DIFFUSE
	);
	ParticleProxy = gEngine->GetRenderingDevice()->CreateParticle();
	// Emit(settings.InitialSize);
}

ParticleEmitter::~ParticleEmitter()
{
	if (Spritesheet)
	{
		ResourceManager<TextureResource>::Release(Spritesheet);
	}
}

void ParticleEmitter::Emit(size_t size)
{
	toEmit += size;
// 	size_t sizeLeft = ParticlesPool.GetFreeBlockCount();
// 	if (size > sizeLeft)
// 	{
// 		gConsole.LogInfo("ParticleEmitter::Emit not enough memory in room (1000)");
// 	}
// 
// 	size_t amount = Clamp(size, (size_t)0, sizeLeft);
// 
// 	// gConsole.LogInfo("ParticleEmitter::Emit emitLen: {}", amount);
// 
// 	while (amount > 0)
// 	{
// 		Particle* p = ParticlesPool.Alloc();
// 		::new(p) Particle();
// 
// 		settings.ParticleInitFunc(p);
// 
// 		--amount;
// 	}
// 
// 	RecreateBufferForProxy();
// 
// 	UpdateDeviceProxy();
}

void ParticleEmitter::Update(World* world)
{

}

void ParticleEmitter::RecreateBufferForProxy()
{
	InstancesTransform.Clear();
	InstancesTransform.Resize(16 * ParticlesPool.GetSize());

	for (int i = 0; i < InstancesTransform.GetSize(); ++i)
	{
		InstancesTransform[i] = 0.0f;
	}

	int transIndx = 0;
	for (Particle& p : ParticlesPool)
	{
		// Scale
		InstancesTransform[transIndx + 0]  = p.Scale.X;
		InstancesTransform[transIndx + 5]  = p.Scale.Y;
		InstancesTransform[transIndx + 10] = p.Scale.Z;
		InstancesTransform[transIndx + 15] = 1.0f;
		// translation
		InstancesTransform[transIndx + 12] = p.Position.X;
		InstancesTransform[transIndx + 13] = p.Position.Y;
		InstancesTransform[transIndx + 14] = p.Position.Z;
		transIndx += 16;
	}
}

void ParticleEmitter::UpdateDeviceProxy()
{
	ParticleProxy->SetContent(*this);
}

// float ParticleEmitter::Random()
// {
// 	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
// }
// 
// float ParticleEmitter::Random(float min, float max)
// {
// 	float rnd = Random();
// 	return Lerp(min, max, rnd);
// }