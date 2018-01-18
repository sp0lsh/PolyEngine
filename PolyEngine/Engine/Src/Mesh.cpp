#include "EnginePCH.hpp"
#include "Mesh.hpp"


#include "TextureResource.hpp"

Poly::Mesh::~Mesh()
{
	if (Mtl.DiffuseTexture)
		ResourceManager<TextureResource>::Release(Mtl.DiffuseTexture);
}

void Poly::Mesh::SetMaterial(const Material & value)
{
	Mtl.SpecularIntensity = value.SpecularIntensity;
	Mtl.SpecularPower = value.SpecularPower;
	Mtl.SpecularColor = value.SpecularColor;
	Mtl.DiffuseTexture = value.DiffuseTexture;
}

void Poly::Mesh::UpdateDeviceProxy()
{
	MeshProxy = gEngine->GetRenderingDevice()->CreateMesh();
	MeshProxy->SetContent(*this);
}
