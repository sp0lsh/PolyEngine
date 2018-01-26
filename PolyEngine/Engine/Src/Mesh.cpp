#include "EnginePCH.hpp"
#include "Mesh.hpp"

#include "TextureResource.hpp"

using namespace Poly;

Mesh::~Mesh()
{
	if (Mtl.DiffuseTexture)
		ResourceManager<TextureResource>::Release(Mtl.DiffuseTexture);
}

void Mesh::SetMaterial(const Material & value)
{
	Mtl.SpecularIntensity = value.SpecularIntensity;
	Mtl.SpecularPower = value.SpecularPower;
	Mtl.SpecularColor = value.SpecularColor;
	Mtl.DiffuseTexture = value.DiffuseTexture;
}

void Mesh::UpdateDeviceProxy()
{
	MeshProxy = gEngine->GetRenderingDevice()->CreateMesh();
	MeshProxy->SetContent(*this);
}
