#include "EnginePCH.hpp"
#include "ParticleEmitter.hpp"

#include <algorithm>

using namespace Poly;

ParticleEmitter::ParticleEmitter(const Settings& settings)
	: settings(settings), ParticlesPool(settings.MaxSize), ToEmit(settings.InitialSize)
{
	Spritesheet = ResourceManager<TextureResource>::Load(
		settings.SpritesheetSettings.SpritePath,
		settings.SpritesheetSettings.Source,
		eTextureUsageType::DIFFUSE
	);
	ParticleProxy = gEngine->GetRenderingDevice()->CreateParticle();
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
	ToEmit += size;
}