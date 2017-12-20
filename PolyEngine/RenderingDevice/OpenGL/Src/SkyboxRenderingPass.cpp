#include "SkyboxRenderingPass.hpp"

#include "GLRenderingDevice.hpp"
#include "GLCubemapDeviceProxy.hpp"

#include <AABox.hpp>
#include <AARect.hpp>
#include <World.hpp>
#include <CameraComponent.hpp>
#include <SkyboxWorldComponent.hpp>

using namespace Poly;

SkyboxRenderingPass::SkyboxRenderingPass(const PrimitiveCube* cube)
	: RenderingPassBase("Shaders/skyboxVert.shader", "Shaders/skyboxFrag.shader"), Cube(cube)
{
	GetProgram().RegisterUniform("mat4", "uMVP");
}

void SkyboxRenderingPass::OnRun(World* world, const CameraComponent* camera, const AARect& /*rect*/, ePassType /*passType = ePassType::GLOBAL*/ )
{
	const SkyboxWorldComponent* SkyboxWorldCmp = world->GetWorldComponent<SkyboxWorldComponent>();
	if (SkyboxWorldCmp == nullptr)
	{
		gConsole.LogInfo("SkyboxRenderingPass::OnRun: SkyboxWorldCmp is NULL");
	}
	else
	{
		RenderSkybox(camera, SkyboxWorldCmp);
	}
}

void SkyboxRenderingPass::RenderSkybox(const CameraComponent* camera, const SkyboxWorldComponent* SkyboxWorldCmp)
{
	// if (SkyboxWorldCmp->GetCubemap().GetState() != eResourceState::VALID
	// 	&& SkyboxWorldCmp->GetPanorama().GetState() != eResourceState::VALID)
	// {
	// 	gConsole.LogWarning("SkyboxRenderingPass::RenderSkybox: Cubemap and Panorama both have invealid resources, aborting!");
	// 	return;
	// }

	GLuint CubemapID = 0;
	GLuint PanoramaID = 0;

	switch (SkyboxWorldCmp->GetSkyboxType())
	{
		case eSkyboxType::CUBEMAP:
			GetProgram().BindProgram(); // Cubemap

			CubemapID = static_cast<const GLCubemapDeviceProxy*>(SkyboxWorldCmp->GetCubemap().GetTextureProxy())->GetTextureID();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapID);
			break;
		
		case eSkyboxType::PANORAMA:
			GetProgram().BindProgram(); // Texture

			// TextureID = static_cast<const GLTextureDeviceProxy*>(SkyboxWorldCmp->GetPanorama().GetTextureProxy())->GetTextureID();

			// glActiveTexture(GL_TEXTURE0);
			// glBindTexture(GL_TEXTURE_2D, TextureID);
			break;

		default:
			ASSERTE(false, "SkyboxRenderingPass::RenderSkybox uknown eSkyboxType");
			break;
	}

	const Matrix projection = camera->GetProjectionMatrix();
	Matrix modelView = Matrix(camera->GetModelViewMatrix());
	// center cube in view space by setting translation to 0 for x, y and z. SetTranslation resets Matrix to identity
	modelView.Data[3] = 0.0f;
	modelView.Data[7] = 0.0f;
	modelView.Data[11] = 0.0f;

	Matrix mvp = projection * modelView;
	GetProgram().SetUniform("uMVP", mvp);

	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);

	glBindVertexArray(Cube->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}
