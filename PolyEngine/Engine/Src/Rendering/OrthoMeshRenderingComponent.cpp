#include <EnginePCH.hpp>

#include <Rendering/Camera/CameraComponent.hpp>
#include <Rendering/OrthoMeshRenderingComponent.hpp>
#include <Resources/ResourceManager.hpp>

using namespace Poly;

RTTI_DEFINE_COMPONENT(::Poly::OrthoMeshRenderingComponent)

OrthoMeshRenderingComponent::OrthoMeshRenderingComponent(const String& meshPath, eResourceSource source)
{
	Mesh = ResourceManager<MeshResource>::Load(meshPath, source);
	if (Mesh) {
		size_t materialsNum = GetMesh()->GetSubMeshes().GetSize();
		Materials.Resize(materialsNum);
		for (size_t i = 0; i < materialsNum; ++i)
		{
			SetMaterial(i, Material());
		}
	}
}

Poly::OrthoMeshRenderingComponent::~OrthoMeshRenderingComponent()
{
	if (Mesh)
		ResourceManager<MeshResource>::Release(Mesh);
}

Optional<AABox> Poly::OrthoMeshRenderingComponent::GetBoundingBox(eEntityBoundingChannel channel)
{
	if (channel != eEntityBoundingChannel::RENDERING || !Mesh)
		return {};
	return Mesh->GetAABox();
}
