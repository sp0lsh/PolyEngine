#include "EnginePCH.hpp"

#include "CameraComponent.hpp"
#include "MeshRenderingComponent.hpp"
#include "ResourceManager.hpp"
#include "Mesh.hpp"

using namespace Poly;

MeshRenderingComponent::MeshRenderingComponent(const String& meshPath, eResourceSource source)
{
	Mesh = ResourceManager<MeshResource>::Load(meshPath, source);
	Materials.Resize(Mesh->GetMeshes().GetSize());
}

MeshRenderingComponent::MeshRenderingComponent(Poly::Mesh* mesh)
{
	Mesh = new MeshResource(mesh);
}

MeshRenderingComponent::~MeshRenderingComponent()
{
	if (Mesh)
		ResourceManager<MeshResource>::Release(Mesh);
}
