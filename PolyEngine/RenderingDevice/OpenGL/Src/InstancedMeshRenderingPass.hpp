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

		static const int instancesLen = 10;

		float instanceTransform[16 * instancesLen];

		const PostprocessQuad* Quad;

		float Random() const;

		float Random(float min, float max) const;
	};
}