#include "EnginePCH.hpp"

#include "CameraComponent.hpp"
#include "MeshRenderingComponent.hpp"
#include "ResourceManager.hpp"

using namespace Poly;

MeshRenderingComponent::MeshRenderingComponent(const String& meshPath, eResourceSource source)
{
	Mesh = ResourceManager<MeshResource>::Load(meshPath, source);
	Materials.Resize(Mesh->GetSubMeshes().GetSize());
}

MeshRenderingComponent::~MeshRenderingComponent()
{
	if (Mesh)
		ResourceManager<MeshResource>::Release(Mesh);
}
