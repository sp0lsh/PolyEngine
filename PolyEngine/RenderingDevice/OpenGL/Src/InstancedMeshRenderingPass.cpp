#include "InstancedMeshRenderingPass.hpp"

#include "GLMeshDeviceProxy.hpp"
#include "GLTextureDeviceProxy.hpp"

#include <World.hpp>
#include <TimeSystem.hpp>
#include <CameraComponent.hpp>
#include <TransformComponent.hpp>
#include <MeshRenderingComponent.hpp>
#include <MovementSystem.hpp>

using namespace Poly;

InstancedMeshRenderingPass::InstancedMeshRenderingPass(const PostprocessQuad* quad)
	: RenderingPassBase("Shaders/instancedVert.shader", "Shaders/instancedFrag.shader"), Quad(quad)
{
	GetProgram().RegisterUniform("float", "uTime");
}

void InstancedMeshRenderingPass::OnRun(World* world, const CameraComponent* camera, const AARect& /*rect*/, ePassType passType = ePassType::GLOBAL)
{
	gConsole.LogInfo("InstancedMeshRenderingPass::OnRun");
	
	float Time = (float)TimeSystem::GetTimerElapsedTime(world, eEngineTimer::GAMEPLAY);

	GetProgram().BindProgram();
	const Matrix& mvp = camera->GetMVP();

	GetProgram().BindProgram();
	GetProgram().SetUniform("uTime", Time);

	glBindVertexArray(Quad->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	
	// Render meshes
	// for (auto componentsTuple : world->IterateComponents<MeshRenderingComponent, TransformComponent>())
	// {
	// 	const MeshRenderingComponent* meshCmp = std::get<MeshRenderingComponent*>(componentsTuple);
	// 	TransformComponent* transCmp = std::get<TransformComponent*>(componentsTuple);
	// 
	// 	if (passType == ePassType::BY_MATERIAL &&
	// 		(meshCmp->IsTransparent() || meshCmp->GetShadingModel() != eShadingModel::UNLIT))
	// 	{
	// 		continue;
	// 	}
	// 
	// 	Vector objPos = transCmp->GetGlobalTranslation();
	// 
	// 	const Matrix& objTransform = transCmp->GetGlobalTransformationMatrix();
	// 	Matrix screenTransform = mvp * objTransform;
	// 	GetProgram().SetUniform("uTransform", objTransform);
	// 	GetProgram().SetUniform("uMVPTransform", screenTransform);
	// 	
	// 	if (passType == ePassType::BY_MATERIAL)
	// 	{
	// 		glPolygonMode(GL_FRONT_AND_BACK, meshCmp->GetIsWireframe() ? GL_LINE : GL_FILL);
	// 	}
	// 
	// 	int i = 0;
	// 	for (const MeshResource::SubMesh* subMesh : meshCmp->GetMesh()->GetSubMeshes())
	// 	{
	// 		PhongMaterial material = meshCmp->GetMaterial(i);
	// 		GetProgram().SetUniform("uColor", material.DiffuseColor);
	// 
	// 		const GLMeshDeviceProxy* meshProxy = static_cast<const GLMeshDeviceProxy*>(subMesh->GetMeshProxy());
	// 		glBindVertexArray(meshProxy->GetVAO());
	// 
	// 		const Poly::TextureResource* DiffuseTexture = subMesh->GetMeshData().GetDiffTexture();
	// 		GLuint TextureID = DiffuseTexture == nullptr
	// 			? FallbackWhiteTexture
	// 			: static_cast<const GLTextureDeviceProxy*>(DiffuseTexture->GetTextureProxy())->GetTextureID();
	// 
	// 		glActiveTexture(GL_TEXTURE0);
	// 		glBindTexture(GL_TEXTURE_2D, TextureID);
	// 
	// 		glDrawElements(GL_TRIANGLES, (GLsizei)subMesh->GetMeshData().GetTriangleCount() * 3, GL_UNSIGNED_INT, NULL);
	// 		glBindTexture(GL_TEXTURE_2D, 0);
	// 		glBindVertexArray(0);
	// 
	// 		++i;
	// 	}
	// 	
	// 	if (passType == ePassType::BY_MATERIAL)
	// 	{
	// 		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// 	}
	// }
}