#include "DebugNormalsRenderingPass.hpp"

#include "GLMeshDeviceProxy.hpp"
#include "GLTextureDeviceProxy.hpp"

#include <World.hpp>
#include <CameraComponent.hpp>
#include <TransformComponent.hpp>
#include <MeshRenderingComponent.hpp>

using namespace Poly;

DebugNormalsRenderingPass::DebugNormalsRenderingPass()
: RenderingPassBase("Shaders/normalsVert.shader", "Shaders/normalsFrag.shader")
{
}

void DebugNormalsRenderingPass::OnRun(World* world, const CameraComponent* camera, const AARect& /*rect*/, ePassType passType = ePassType::GLOBAL)
{
	GetProgram().BindProgram();
	const Matrix& mvp = camera->GetMVP();
	
	// Render meshes
	for (auto componentsTuple : world->IterateComponents<MeshRenderingComponent, TransformComponent>())
	{
		const MeshRenderingComponent* meshCmp = std::get<MeshRenderingComponent*>(componentsTuple);
		TransformComponent* transCmp = std::get<TransformComponent*>(componentsTuple);

		if (passType == ePassType::BY_MATERIAL &&
			(meshCmp->IsTransparent() || meshCmp->GetShadingModel() != eShadingModel::LIT))
		{
		 	continue;
		}

		Vector objPos = transCmp->GetGlobalTranslation();

		const Matrix& objTransform = transCmp->GetGlobalTransformationMatrix();
		Matrix screenTransform = mvp * objTransform;
		GetProgram().SetUniform("uTransform", objTransform);
		GetProgram().SetUniform("uMVPTransform", screenTransform);
		
		glPolygonMode(GL_FRONT_AND_BACK, meshCmp->GetIsWireframe() ? GL_LINE : GL_FILL);

		int i = 0;
		for (const Mesh* subMesh : meshCmp->GetMesh()->GetMeshes())
		{
			const GLMeshDeviceProxy* meshProxy = static_cast<const GLMeshDeviceProxy*>(subMesh->GetMeshProxy());

			glBindVertexArray(meshProxy->GetVAO());

			glDrawElements(GL_TRIANGLES, (GLsizei)subMesh->GetTriangleCount() * 3, GL_UNSIGNED_INT, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);

			++i;
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
