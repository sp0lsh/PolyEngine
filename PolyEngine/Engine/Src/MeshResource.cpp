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
		SubMeshes.PushBack(LoadMesh(path, scene->mMeshes[i], scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]));
	}
}

Poly::MeshResource::~MeshResource()
{
	for (Mesh* subMesh : SubMeshes)
	{
		delete subMesh;
	}
}

Mesh* Poly::MeshResource::LoadMesh(const String& path, aiMesh* rawMesh, aiMaterial* rawMaterial)
{
	Mesh* MeshData = new Mesh();

	if (rawMesh->HasPositions()) {
		MeshData->Positions.Resize(rawMesh->mNumVertices);
		for (unsigned int i = 0; i < rawMesh->mNumVertices; ++i) {
			MeshData->Positions[i].X  = rawMesh->mVertices[i].x;
			MeshData->Positions[i].Y = rawMesh->mVertices[i].y;
			MeshData->Positions[i].Z = rawMesh->mVertices[i].z;
		}
	}

	if (rawMesh->HasTextureCoords(0)) {
		MeshData->TextCoords.Resize(rawMesh->mNumVertices);
		for (unsigned int i = 0; i < rawMesh->mNumVertices; ++i) {
			MeshData->TextCoords[i].X = rawMesh->mTextureCoords[0][i].x;
			MeshData->TextCoords[i].Y = rawMesh->mTextureCoords[0][i].y;
		}
	}

	if (rawMesh->HasNormals()) {
		MeshData->Normals.Resize(rawMesh->mNumVertices);
		for (unsigned int i = 0; i < rawMesh->mNumVertices; ++i) {
			MeshData->Normals[i].X = rawMesh->mNormals[i].x;
			MeshData->Normals[i].Y = rawMesh->mNormals[i].y;
			MeshData->Normals[i].Z = rawMesh->mNormals[i].z;
		}
	}

	if (rawMesh->HasFaces()) {
		MeshData->Indices.Resize(rawMesh->mNumFaces * 3);
		for (unsigned int i = 0; i < rawMesh->mNumFaces; ++i) {
			MeshData->Indices[i * 3] = rawMesh->mFaces[i].mIndices[0];
			MeshData->Indices[i * 3 + 1] = rawMesh->mFaces[i].mIndices[1];
			MeshData->Indices[i * 3 + 2] = rawMesh->mFaces[i].mIndices[2];
		}
	}

	MeshData->UpdateDeviceProxy();

	gConsole.LogDebug(
		"Loaded mesh entry: {} with {} vertices, {} faces and parameters: "
		"pos[{}], tex_coord[{}], norm[{}], faces[{}]",
		rawMesh->mName.C_Str(), MeshData->Positions.GetSize(), rawMesh->mNumFaces,
		rawMesh->HasPositions() ? "on" : "off",
		rawMesh->HasTextureCoords(0) ? "on" : "off",
		rawMesh->HasNormals() ? "on" : "off", rawMesh->HasFaces() ? "on" : "off");

	// Material loading
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

		MeshData->Mtl.DiffuseTexture = ResourceManager<TextureResource>::Load(textPath, eResourceSource::NONE);
		if (!MeshData->Mtl.DiffuseTexture) {
			gConsole.LogError("Failed to load diffuse texture: {}", fullPath);
		} else {
			gConsole.LogDebug("Succeded to load diffuse texture: {}", fullPath);
		}
	} else {
		gConsole.LogError("Failed to load diffuse texture for material: {}", path);
		MeshData->Mtl.DiffuseTexture = nullptr;
	}

	// Material params loading
	if (rawMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, MeshData->Mtl.SpecularIntensity) != AI_SUCCESS) {
		MeshData->Mtl.SpecularIntensity = 0.0f;
		gConsole.LogError("Error reading specular intensity in {}", path);
	}
	if (rawMaterial->Get(AI_MATKEY_SHININESS, MeshData->Mtl.SpecularPower) != AI_SUCCESS) {
		MeshData->Mtl.SpecularPower = 0.0f;
		gConsole.LogError("Error reading specular power in {}", path);
	}
	aiColor3D color;
	rawMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
	MeshData->Mtl.SpecularColor = Color(color.r, color.b, color.g);

	gConsole.LogDebug("Specular params: {}, {}, [{},{},{}]",
		MeshData->Mtl.SpecularIntensity,
		MeshData->Mtl.SpecularPower,
		MeshData->Mtl.SpecularColor.R,
		MeshData->Mtl.SpecularColor.G,
		MeshData->Mtl.SpecularColor.B);

	return MeshData;
}
