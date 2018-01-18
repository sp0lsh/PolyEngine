#pragma once

#include <Dynarray.hpp>
#include <EnumUtils.hpp>
#include <Color.hpp>

#include "ResourceBase.hpp"
#include "TextureResource.hpp"
#include "Mesh.hpp"

struct aiMesh;
struct aiMaterial;

typedef unsigned int GLuint;
 
namespace Poly
{
	class ENGINE_DLLEXPORT MeshResource : public ResourceBase
	{
	public:
		
		MeshResource(const String& path);
		MeshResource(Mesh* mesh);
		virtual ~MeshResource();

		const Dynarray<Mesh*>& GetMeshes() const { return Meshes; }

	private:
		Mesh* LoadMesh(const String& path, aiMesh* rawMesh, aiMaterial* rawMaterial);
		Dynarray<Mesh*> Meshes;
	};
}