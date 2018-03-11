#include "ParticlesRenderingPass.hpp"

#include "GLMeshDeviceProxy.hpp"
#include "GLTextureDeviceProxy.hpp"

#include <World.hpp>
#include <TimeSystem.hpp>
#include <CameraComponent.hpp>
#include <ParticleComponent.hpp>
#include <MeshRenderingComponent.hpp>
#include <MovementSystem.hpp>

using namespace Poly;

ParticlesRenderingPass::ParticlesRenderingPass()
	: RenderingPassBase("Shaders/instancedVert.shader", "Shaders/instancedFrag.shader")
{
	GetProgram().RegisterUniform("float", "uTime");
	GetProgram().RegisterUniform("mat4", "uScreenFromView");
	GetProgram().RegisterUniform("mat4", "uViewFromWorld");
	GetProgram().RegisterUniform("mat4", "uWorldFromModel");
	GetProgram().RegisterUniform("vec4", "uColor");
}

void ParticlesRenderingPass::OnRun(World* world, const CameraComponent* camera, const AARect& /*rect*/, ePassType passType = ePassType::GLOBAL)
{
	float Time = (float)TimeSystem::GetTimerElapsedTime(world, eEngineTimer::GAMEPLAY);
	const Matrix& ViewFromWorld = camera->GetViewFromWorld();
	const Matrix& ScreenFromView = camera->GetScreenFromView();

	glDisable(GL_CULL_FACE);

	GetProgram().BindProgram();
	GetProgram().SetUniform("uTime", Time);
	GetProgram().SetUniform("uScreenFromView", ScreenFromView);

	for (auto componentsTuple : world->IterateComponents<ParticleComponent>())
	{
		/*
		struct ENGINE_DLLEXPORT Settings
		{
			eSimulationSpace SimulationSpace = eSimulationSpace::WORLD_SPACE;
			struct ENGINE_DLLEXPORT SpritesheetSettings
			{
				# Vector2f SubImages = Vector2f(4.0, 4.0);
				# Color Color = Color::WHITE;
				float StartFrame = 0.0f;
				float Speed = 1.0f;
				float IsRandom = 0.0f;
			};
			# float Speed = 1.0f;
			# Color Color = Color::WHITE;
		};
		*/

		const ParticleComponent* particleCmp = std::get<ParticleComponent*>(componentsTuple);
		const EntityTransform& transform = particleCmp->GetTransform();
		const Matrix& WorldFromModel = particleCmp->GetEmitter()->GetSettings().SimulationSpace == ParticleEmitter::eSimulationSpace::LOCAL_SPACE
			? transform.GetWorldFromModel()
			: Matrix();
		// Matrix ViewFromModel = ViewFromWorld * WorldFromModel;
		
		GetProgram().SetUniform("uViewFromWorld", ViewFromWorld);
		GetProgram().SetUniform("uWorldFromModel", WorldFromModel);
		GetProgram().SetUniform("uColor", particleCmp->GetEmitter()->GetSettings().Color);
		GetProgram().SetUniform("uSpeed", particleCmp->GetEmitter()->GetSettings().Speed);

		int partileLen = particleCmp->GetEmitter()->GetInstancesCount();
		const TextureResource* Texture = particleCmp->GetEmitter()->GetSpritesheet();
		const GLParticleDeviceProxy* particleProxy = static_cast<const GLParticleDeviceProxy*>(particleCmp->GetEmitter()->GetParticleProxy());
		GLuint particleVAO = particleProxy->GetVAO();

		GLuint TextureID = Texture == nullptr
			? FallbackWhiteTexture
			: static_cast<const GLTextureDeviceProxy*>(Texture->GetTextureProxy())->GetTextureID();

		GetProgram().SetUniform("uHasSprite", Texture == nullptr ? 0.0f : 1.0f );

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureID);

		glBindVertexArray(particleVAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, partileLen);
		glBindVertexArray(0);
		CHECK_GL_ERR();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glEnable(GL_CULL_FACE);
}
