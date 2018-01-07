#pragma once

#include "RenderingPassBase.hpp"
#include "GLShaderProgram.hpp"
#include "PostprocessQuad.hpp"

namespace Poly
{
	class World;
	class RenderingTargetBase;
	class GLTextureDeviceProxy;
	class AARect;

	//------------------------------------------------------------------------------
	class InstancedMeshRenderingPass : public RenderingPassBase
	{
	public:
		InstancedMeshRenderingPass(const PostprocessQuad* quad);

	protected:
		void OnRun(World* world, const CameraComponent* camera, const AARect& rect, ePassType passType) override;

	private:

		unsigned int quadVAO, quadVBO;
		unsigned int instanceVBO;

		float translation[200];

		const PostprocessQuad* Quad;
	};
}