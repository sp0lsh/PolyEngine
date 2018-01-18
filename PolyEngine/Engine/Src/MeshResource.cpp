#include "EnginePCH.hpp"

#include "MeshResource.hpp"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

using namespace Poly;

MeshResource::MeshResource(const String& path)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path.GetCStr(), aiProcessPreset_TargetRealtime_Fast);

	if (!scene) {
		gConsole.LogError("Error Importing Asset: {}", importer.GetErrorString());
		throw ResourceLoadFailedException();
	}

	gConsole.LogDebug("Loading model {} sucessfull.", path);
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		Meshes.PushBack(LoadMesh(path, scene->mMeshes[i], scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]));
	}
}

MeshResource::MeshResource(Mesh* mesh)
{
	Meshes.PushBack(mesh);
}

Poly::MeshResource::~MeshResource()
{
	for (Mesh* Mesh : Meshes)
	{
		delete Mesh;
	}
}

Mesh* Poly::MeshResource::LoadMesh(const String& path, aiMesh* rawMesh, aiMaterial* rawMaterial)
{
	Mesh* MeshData = new Mesh();
	Dynarray<Vector3f> Positions;
	Dynarray<Vector3f> Normals;
	Dynarray<Vector2f> UVs;
	Dynarray<uint32_t> Indices;


	if (rawMesh->HasPositions()) {
		Positions.Resize(rawMesh->mNumVertices);
		for (unsigned int i = 0; i < rawMesh->mNumVertices; ++i) {
			Positions[i].X  = rawMesh->mVertices[i].x;
			Positions[i].Y = rawMesh->mVertices[i].y;
			Positions[i].Z = rawMesh->mVertices[i].z;
		}
	}

	if (rawMesh->HasTextureCoords(0)) {
		UVs.Resize(rawMesh->mNumVertices);
		for (unsigned int i = 0; i < rawMesh->mNumVertices; ++i) {
			UVs[i].X = rawMesh->mTextureCoords[0][i].x;
			UVs[i].Y = rawMesh->mTextureCoords[0][i].y;
		}
	}

	if (rawMesh->HasNormals()) {
		Normals.Resize(rawMesh->mNumVertices);
		for (unsigned int i = 0; i < rawMesh->mNumVertices; ++i) {
			Normals[i].X = rawMesh->mNormals[i].x;
			Normals[i].Y = rawMesh->mNormals[i].y;
			Normals[i].Z = rawMesh->mNormals[i].z;
		}
	}

	if (rawMesh->HasFaces()) {
		Indices.Resize(rawMesh->mNumFaces * 3);
		for (unsigned int i = 0; i < rawMesh->mNumFaces; ++i) {
			Indices[i * 3] = rawMesh->mFaces[i].mIndices[0];
			Indices[i * 3 + 1] = rawMesh->mFaces[i].mIndices[1];
			Indices[i * 3 + 2] = rawMesh->mFaces[i].mIndices[2];
		}
	}

	MeshData->SetPositions(Positions);
	MeshData->SetTextCoords(UVs);
	MeshData->SetIndicies(Indices);
	MeshData->SetNormals(Normals);

	MeshData->UpdateDeviceProxy();


	gConsole.LogDebug(
		"Loaded mesh entry: {} with {} vertices, {} faces and parameters: "
		"pos[{}], tex_coord[{}], norm[{}], faces[{}]",
		rawMesh->mName.C_Str(), MeshData->GetPositions().GetSize(), rawMesh->mNumFaces,
		rawMesh->HasPositions() ? "on" : "off",
		rawMesh->HasTextureCoords(0) ? "on" : "off",
		rawMesh->HasNormals() ? "on" : "off", rawMesh->HasFaces() ? "on" : "off");

	// Material loading
	Mesh::Material material;
	aiString texPath;
	if (rawMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
	{
		//TODO load textures, this requires Path class
		// temporary code for extracting path
		std::string tmpPath = std::string(path.GetCStr());
		std::replace(tmpPath.begin(), tmpPath.end(), '\\', '/'); // replace all '\' to '/', fix for linux machines
		std::string fullPath = tmpPath.substr(0, tmpPath.rfind('/')+1) + std::string(texPath.C_Str());
		std::replace(fullPath.begin(), fullPath.end(), '\\', '/'); // replace all '\' to '/', fix for linux machines
		String textPath(fullPath.c_str());
		// end temporary code for extracting path

		material.DiffuseTexture = ResourceManager<TextureResource>::Load(textPath, eResourceSource::NONE);
		if (!material.DiffuseTexture) {
			gConsole.LogError("Failed to load diffuse texture: {}", fullPath);
		} else {
			gConsole.LogDebug("Succeded to load diffuse texture: {}", fullPath);
		}
	} else {
		gConsole.LogError("Failed to load diffuse texture for material: {}", path);
		material.DiffuseTexture = nullptr;
	}

	// Material params loading
	if (rawMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, material.SpecularIntensity) != AI_SUCCESS) {
		material.SpecularIntensity = 0.0f;
		gConsole.LogError("Error reading specular intensity in {}", path);
	}
	if (rawMaterial->Get(AI_MATKEY_SHININESS, material.SpecularPower) != AI_SUCCESS) {
		material.SpecularPower = 0.0f;
		gConsole.LogError("Error reading specular power in {}", path);
	}
	aiColor3D color;
	rawMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
	material.SpecularColor = Color(color.r, color.b, color.g);

	MeshData->SetMaterial(material);

	gConsole.LogDebug("Specular params: {}, {}, [{},{},{}]",
		MeshData->GetMaterial().SpecularIntensity,
		MeshData->GetMaterial().SpecularPower,
		MeshData->GetMaterial().SpecularColor.R,
		MeshData->GetMaterial().SpecularColor.G,
		MeshData->GetMaterial().SpecularColor.B);

	return MeshData;
}
