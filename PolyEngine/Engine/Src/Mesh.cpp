#include "EnginePCH.hpp"
#include "Mesh.hpp"


#include "TextureResource.hpp"

Poly::Mesh::~Mesh()
{
	if (Mtl.DiffuseTexture)
		ResourceManager<TextureResource>::Release(Mtl.DiffuseTexture);
}

void Poly::Mesh::UpdateDeviceProxy()
{
	MeshProxy = gEngine->GetRenderingDevice()->CreateMesh();
	MeshProxy->SetContent(*this);
}
