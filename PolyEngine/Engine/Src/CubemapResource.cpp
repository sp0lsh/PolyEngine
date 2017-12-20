#include "EnginePCH.hpp"

#include "CubemapResource.hpp"
#include "ResourceManager.hpp"
#include "SOIL/SOIL.h"

using namespace Poly;

CubemapResource::CubemapResource(const Dynarray<String> paths)
{
	State = paths.GetSize() == 6 ? eResourceState::VALID : eResourceState::INVALID;
	if (State == eResourceState::INVALID)
	{
		gConsole.LogWarning("CubemapResource::CubemapResource parths need to have 6 elements");
		return;
	}

	gConsole.LogInfo("CubemapResource::CubemapResource path:{}", paths[0]);

	Images.Reserve(6);
	for (size_t i = 0; i < paths.GetSize(); ++i) 
	{
		String absolutePath = gAssetsPathConfig.GetAssetsPath(eResourceSource::ENGINE) + paths[i];
		Images.PushBack(LoadImage(absolutePath));
	}

	TextureProxy = gEngine->GetRenderingDevice()->CreateCubemap(Width, Height);

	for (size_t i = 0; i < Images.GetSize(); ++i)
	{
		TextureProxy->SetContent((unsigned int)i, Images[i]);
	}
	
	State = eResourceState::VALID;
}

unsigned char* CubemapResource::LoadImage(const String& path)
{
	unsigned char* image;
	Channels = 3;

	int fileChannels;
	int fileWidth;
	int fileHeight;
	image = SOIL_load_image(path.GetCStr(), &fileWidth, &fileHeight, &fileChannels, SOIL_LOAD_RGB);
	
	if (image == nullptr)
	{
		State = eResourceState::INVALID;
		gConsole.LogWarning("CubemapResource::LoadImage failed! path:{}", path);
	}
	else
	{
		gConsole.LogInfo("CubemapResource::LoadImage {}x{}:{} path:{}", fileWidth, fileHeight, fileChannels, path);

		Width = std::max(Width, fileWidth);
		Height = std::max(Height, fileHeight);
	}
	return image;
}

CubemapResource::~CubemapResource()
{
	gConsole.LogInfo("CubemapResource::~CubemapResource");

	for(size_t i = 0; i < Images.GetSize(); ++i)
	{
		SOIL_free_image_data(Images[i]);
	}
}