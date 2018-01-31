#include "SpritesheetRenderingPass.hpp"

#include "GLRenderingDevice.hpp"
#include "GLMeshDeviceProxy.hpp"
#include "GLTextureDeviceProxy.hpp"

#include <AABox.hpp>
#include <World.hpp>
#include <TimeSystem.hpp>
#include <CameraComponent.hpp>
#include <MovementSystem.hpp>
#include <SpritesheetComponent.hpp>
#include <PostprocessSettingsComponent.hpp>

using namespace Poly;

SpritesheetRenderingPass::SpritesheetRenderingPass(const PostprocessQuad* quad)
	: RenderingPassBase("Shaders/spritesheetVert.shader", "Shaders/spritesheetFrag.shader"), Quad(quad)
{
	GetProgram().RegisterUniform("float", "uTime");
	GetProgram().RegisterUniform("mat4", "uP");
	GetProgram().RegisterUniform("mat4", "uMV");
	GetProgram().RegisterUniform("vec2", "uScale");
	GetProgram().RegisterUniform("vec2", "uSubImages");
	GetProgram().RegisterUniform("float", "uStartFrame");
	GetProgram().RegisterUniform("float", "uSpeed");
	GetProgram().RegisterUniform("vec4", "uColor");
}

void SpritesheetRenderingPass::OnRun(World* world, const CameraComponent* camera, const AARect& /*rect*/, ePassType /*passType = ePassType::GLOBAL*/ )
{
	float Time = (float)TimeSystem::GetTimerElapsedTime(world, eEngineTimer::GAMEPLAY);
	const Matrix& mv = camera->GetModelViewMatrix();
	const Matrix& p = camera->GetProjectionMatrix();

	glDisable(GL_CULL_FACE);

	GetProgram().BindProgram();
	GetProgram().SetUniform("uTime", Time);
	GetProgram().SetUniform("uP", p);

	// Render meshes
	for (auto componentsTuple : world->IterateComponents<SpritesheetComponent>())
	{
		const SpritesheetComponent* spritesheetCmp = std::get<SpritesheetComponent*>(componentsTuple);
		const SpritesheetComponent::Settings settings = spritesheetCmp->GetSettings();
		const EntityTransform& transform = spritesheetCmp->GetTransform();
		const Matrix& objTransform = transform.GetGlobalTransformationMatrix();
		Matrix screenTransform = mv * objTransform;

		// gConsole.LogError("SpritesheetRenderingPass::OnRun subImages: {}, uSpeed: {}, uStartFrame: {}",
		// 	settings.SubImages, settings.Speed, settings.StartFrame);

		GetProgram().SetUniform("uMV", screenTransform);
		GetProgram().SetUniform("uScale", objTransform.m00, objTransform.m11);
		GetProgram().SetUniform("uSubImages", settings.SubImages.X, settings.SubImages.Y);
 		GetProgram().SetUniform("uSpeed", (float)(settings.Speed));
		GetProgram().SetUniform("uColor", settings.Color);

		float startFrame = settings.IsRandom ? RandomRange(0.0f, settings.SubImages.X * settings.SubImages.Y) : settings.StartFrame;
 		GetProgram().SetUniform("uStartFrame", startFrame);

		const TextureResource* Spritesheet = spritesheetCmp->GetSpritesheet();
		GLuint TextureID = Spritesheet == nullptr
			? FallbackWhiteTexture
			: static_cast<const GLTextureDeviceProxy*>(Spritesheet->GetTextureProxy())->GetTextureID();
			
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureID);

		glBindVertexArray(Quad->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	glEnable(GL_CULL_FACE);
}
