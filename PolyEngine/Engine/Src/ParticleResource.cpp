#include "EnginePCH.hpp"

#include "ParticleResource.hpp"

// #include <assimp/cimport.h>
// #include <assimp/postprocess.h>
// #include <assimp/scene.h>
// #include <assimp/Importer.hpp>

using namespace Poly;

ParticleResource::ParticleResource()
{
	ParticleProxy = gEngine->GetRenderingDevice()->CreateParticle();
	ParticleProxy->SetContent(ParticleData);

	// Assimp::Importer importer;
	// const aiScene *scene = importer.ReadFile(path.GetCStr(), aiProcessPreset_TargetRealtime_Fast);
	// 
	// if (!scene) {
	// 	gConsole.LogError("Error Importing Asset: {}", importer.GetErrorString());
	// 	throw ResourceLoadFailedException();
	// }
	// 
	// gConsole.LogDebug("Loading model {} sucessfull.", path);
	// for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
	// 	SubMeshes.PushBack(new SubMesh(path, scene->mMeshes[i], scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]));
	// }
}

ParticleResource::~ParticleResource()
{
	// for (SubMesh* subMesh : SubMeshes)
	// {
	// 	delete subMesh;
	// }
}