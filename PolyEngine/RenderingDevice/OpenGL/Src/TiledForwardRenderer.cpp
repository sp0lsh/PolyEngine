#include "TiledForwardRenderer.hpp"

#include <math.h>

#include "Configs/AssetsPathConfig.hpp"

#include "GLRenderingDevice.hpp"
#include "Proxy/GLMeshDeviceProxy.hpp"
#include "Pipeline/RenderingPassBase.hpp"
#include "Common/GLShaderProgram.hpp"
#include "Common/PrimitiveCube.hpp"
#include "Common/PrimitiveQuad.hpp"

#include <ECS/World.hpp>
#include <Math/BasicMath.hpp>
#include <Math/AARect.hpp>
#include <Rendering/Camera/CameraComponent.hpp>
#include <Rendering/Lighting/LightSourceComponent.hpp>
#include <Rendering/MeshRenderingComponent.hpp>
#include <Movement//MovementSystem.hpp>
#include <Time/TimeWorldComponent.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "Common/stb_image.hpp"

using namespace Poly;

TiledForwardRenderer::TiledForwardRenderer(GLRenderingDevice* rdi)
	: IRendererInterface(rdi),
	DepthShader("Shaders/depth.vert.glsl", "Shaders/depth.frag.glsl"),
	LightCullingShader("Shaders/light_culling.comp.glsl"),
	DebugQuadDepthPrepassShader("Shaders/debugQuadDepthPrepass.vert.glsl", "Shaders/debugQuadDepthPrepass.frag.glsl"),
	DebugLightAccumShader("Shaders/debugLightAccum.vert.glsl", "Shaders/debugLightAccum.frag.glsl"),
	LightAccumulationShader("Shaders/lightAccumulation.vert.glsl", "Shaders/lightAccumulation.frag.glsl"),
	HDRShader("Shaders/hdr.vert.glsl", "Shaders/hdr.frag.glsl"),
	SSAOShader("Shaders/hdr.vert.glsl", "Shaders/ssao.frag.glsl"),
	GammaShader("Shaders/hdr.vert.glsl", "Shaders/gamma.frag.glsl"),
	LinearizeDepthShader("Shaders/hdr.vert.glsl", "Shaders/linearizeDepth.frag.glsl"),
	IntegrateBRDFShader("Shaders/hdr.vert.glsl", "Shaders/integrateBRDF.frag.glsl"),
	ParticleShader("Shaders/instanced.vert.glsl", "Shaders/instanced.frag.glsl"),
	TranslucentShader("Shaders/transparent.vert.glsl", "Shaders/transparent.frag.glsl"),
	SkyboxShader("Shaders/skybox.vert.glsl", "Shaders/skybox.frag.glsl"),
	EquiToCubemapShader("Shaders/equiHdr.vert.glsl", "Shaders/equiHdr.frag.glsl"),
	EquirectangularToCubemapShader("Shaders/equiToCubemap.vert.glsl", "Shaders/equiToCubemap.frag.glsl"),
	CubemapIrradianceShader("Shaders/cubemapIrradiance.vert.glsl", "Shaders/cubemapIrradiance.frag.glsl"),
	PrefilterCubemapShader("Shaders/prefilterCubemap.vert.glsl", "Shaders/prefilterCubemap.frag.glsl"),
	Text2DShader("Shaders/text2D.vert.glsl", "Shaders/text2D.frag.glsl"),
	EditorDebugShader("Shaders/debug.vert.glsl", "Shaders/debug.frag.glsl")
{
	LightAccumulationShader.RegisterUniform("vec4", "uViewPosition");
	LightAccumulationShader.RegisterUniform("mat4", "uClipFromModel");
	LightAccumulationShader.RegisterUniform("mat4", "uWorldFromModel");
	LightAccumulationShader.RegisterUniform("vec4", "uMaterial.Emissive");
	LightAccumulationShader.RegisterUniform("vec4", "uMaterial.Albedo");
	LightAccumulationShader.RegisterUniform("float", "uMaterial.Roughness");
	LightAccumulationShader.RegisterUniform("float", "uMaterial.Metallic");
	for (int i = 0; i < 8; ++i)
	{
		String baseName = String("uDirectionalLight[") + String::From(i) + String("].");
		LightAccumulationShader.RegisterUniform("vec4", baseName + "ColorIntensity");
		LightAccumulationShader.RegisterUniform("vec4", baseName + "Direction");
	}
	LightAccumulationShader.RegisterUniform("int", "uDirectionalLightCount");
	LightAccumulationShader.RegisterUniform("int", "uLightCount");
	LightAccumulationShader.RegisterUniform("int", "uWorkGroupsX");
	LightAccumulationShader.RegisterUniform("int", "uWorkGroupsY");

	SkyboxShader.RegisterUniform("mat4", "uClipFromWorld");
	SkyboxShader.RegisterUniform("float", "uTime");

	TranslucentShader.RegisterUniform("vec4", "uViewPosition");
	TranslucentShader.RegisterUniform("mat4", "uClipFromModel");
	TranslucentShader.RegisterUniform("mat4", "uWorldFromModel");
	TranslucentShader.RegisterUniform("vec4", "uMaterial.Ambient");
	TranslucentShader.RegisterUniform("vec4", "uMaterial.Diffuse");
	TranslucentShader.RegisterUniform("vec4", "uMaterial.Specular");
	TranslucentShader.RegisterUniform("float", "uMaterial.Shininess");
	for (int i = 0; i < 8; ++i)
	{
		String baseName = String("uDirectionalLight[") + String::From(i) + String("].");
		TranslucentShader.RegisterUniform("vec4", baseName + "ColorIntensity");
		TranslucentShader.RegisterUniform("vec4", baseName + "Direction");
	}
	TranslucentShader.RegisterUniform("int", "uDirectionalLightCount");
	
	ParticleShader.RegisterUniform("float", "uTime");
	ParticleShader.RegisterUniform("mat4", "uScreenFromView");
	ParticleShader.RegisterUniform("mat4", "uViewFromWorld");
	ParticleShader.RegisterUniform("mat4", "uWorldFromModel");
	ParticleShader.RegisterUniform("vec4", "uColor");

	LinearizeDepthShader.RegisterUniform("float", "uNear");
	LinearizeDepthShader.RegisterUniform("float", "uFar");

	EditorDebugShader.RegisterUniform("mat4", "uMVP");
}

void TiledForwardRenderer::Init()
{
	gConsole.LogInfo("TiledForwardRenderer::Init");
	
	const ScreenSize screenSize = RDI->GetScreenSize();

	gConsole.LogInfo("TiledForwardRenderer::Init SCREEN_SIZE: ({},{})", screenSize.Width, screenSize.Height);

	LoadHDR();

	CaptureCubemap();

	CaptureDifuseIrradiance();

	CaptureSpecularPrefilteredMap();

	CapturePreintegratedBRDF();

	CreateRenderTargets(screenSize);

	CreateLightBuffers(screenSize);

	SetupLightsBufferFromScene();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void TiledForwardRenderer::LoadHDR()
{
	// TODO: move to texture resource
	String absoluteHDRPath = gAssetsPathConfig.GetAssetsPath(eResourceSource::GAME) + "HDR/HDR.hdr";
	gConsole.LogInfo("TiledForwardRenderer::Init absoluteHDRPath: {}", absoluteHDRPath);
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float *data = stbi_loadf(absoluteHDRPath.GetCStr(), &width, &height, &nrComponents, 0);
	if (data)
	{
		glGenTextures(1, &HDRTexture);
		glBindTexture(GL_TEXTURE_2D, HDRTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		gConsole.LogInfo("TiledForwardRenderer::Init Failed to load HDR image.");
	}
}

void TiledForwardRenderer::CaptureCubemap()
{
	// create cubemap resource for capture
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap create cubemap resource");
	glGenTextures(1, &EnvCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemap);
	for (GLuint i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap cubemap resource created");

	// Create FBO for HDR capture
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap Create capture FBO");
	GLuint captureFBO;
	GLuint captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap Capture FBO created");

	// Projection
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap create matrices");
	Matrix uClipFromView;
	uClipFromView.SetPerspective(90.0_deg, 1.0f, 0.1f, 10.0f);

	Matrix ViewFromModel[] =
	{
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector( 1.0f,  0.0f,  0.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(-1.0f,  0.0f,  0.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector( 0.0f, -1.0f,  0.0f), Vector(0.0f,  0.0f, -1.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector( 0.0f,  1.0f,  0.0f), Vector(0.0f,  0.0f,  1.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector( 0.0f,  0.0f,  1.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector( 0.0f,  0.0f, -1.0f), Vector(0.0f, -1.0f,  0.0f))
	};
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap matrices created");
	
	// convert HDR equirectangular environment map to cubemap equivalent
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap start cubemap rendering loop");
	EquirectangularToCubemapShader.BindProgram();
	EquirectangularToCubemapShader.SetUniform("uEquirectangularMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, HDRTexture);
	 
	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap cubemap rendering: {}", i);
		EquirectangularToCubemapShader.SetUniform("uClipFromModel", uClipFromView * ViewFromModel[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, EnvCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		 
		// renderCube(); // renders a 1x1 cube
		glBindVertexArray(RDI->PrimitivesCube->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap cubemap rendered");

	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap generate env cubemap mipmaps");
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap env cubemap mipmaps generated");
}

void TiledForwardRenderer::CaptureDifuseIrradiance()
{
	glGenTextures(1, &IrradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, IrradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Create FBO for HDR capture
	gConsole.LogInfo("TiledForwardRenderer::CaptureIrradiance Create capture FBO");
	GLuint captureFBO;
	GLuint captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	gConsole.LogInfo("TiledForwardRenderer::CaptureIrradiance Capture FBO created");

	// Projection
	gConsole.LogInfo("TiledForwardRenderer::CaptureIrradiance create matrices");
	Matrix uClipFromView;
	uClipFromView.SetPerspective(90.0_deg, 1.0f, 0.1f, 10.0f);

	Matrix ViewFromModel[] =
	{
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(1.0f,  0.0f,  0.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(-1.0f,  0.0f,  0.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, -1.0f,  0.0f), Vector(0.0f,  0.0f, -1.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f,  1.0f,  0.0f), Vector(0.0f,  0.0f,  1.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f,  0.0f,  1.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f,  0.0f, -1.0f), Vector(0.0f, -1.0f,  0.0f))
	};
	gConsole.LogInfo("TiledForwardRenderer::CaptureIrradiance matrices created");

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	CubemapIrradianceShader.BindProgram();
	CubemapIrradianceShader.SetUniform("uEnvCubemap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemap);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		CubemapIrradianceShader.SetUniform("uClipFromModel", uClipFromView * ViewFromModel[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, IrradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(RDI->PrimitivesCube->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::CaptureSpecularPrefilteredMap()
{
	gConsole.LogInfo("TiledForwardRenderer::CaptureSpecularPrefilteredMap create cubemap resource");
	glGenTextures(1, &PrefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, PrefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	gConsole.LogInfo("TiledForwardRenderer::CaptureSpecularPrefilteredMap cubemap resource created");

	// Create FBO for HDR capture
	gConsole.LogInfo("TiledForwardRenderer::CaptureSpecularPrefilteredMap Create capture FBO");
	GLuint captureFBO;
	GLuint captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	gConsole.LogInfo("TiledForwardRenderer::CaptureSpecularPrefilteredMap Capture FBO created");

	// Projection
	gConsole.LogInfo("TiledForwardRenderer::CaptureSpecularPrefilteredMap create matrices");
	Matrix uClipFromView;
	uClipFromView.SetPerspective(90.0_deg, 1.0f, 0.1f, 10.0f);

	Matrix ViewFromModel[] =
	{
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(1.0f,  0.0f,  0.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(-1.0f,  0.0f,  0.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f, -1.0f,  0.0f), Vector(0.0f,  0.0f, -1.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f,  1.0f,  0.0f), Vector(0.0f,  0.0f,  1.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f,  0.0f,  1.0f), Vector(0.0f, -1.0f,  0.0f)),
		Matrix(Vector(0.0f, 0.0f, 0.0f), Vector(0.0f,  0.0f, -1.0f), Vector(0.0f, -1.0f,  0.0f))
	};
	gConsole.LogInfo("TiledForwardRenderer::CaptureSpecularPrefilteredMap matrices created");

	gConsole.LogInfo("TiledForwardRenderer::CaptureSpecularPrefilteredMap capture prefiltered cubemap");
	PrefilterCubemapShader.BindProgram();
	PrefilterCubemapShader.SetUniform("uEnvironmentMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		// unsigned int mipWidth = 128 * std::pow(0.5, mip);
		// unsigned int mipHeight = 128 * std::pow(0.5, mip);
		unsigned int mipWidth = 512 * (unsigned int)std::pow(0.5f, mip);
		unsigned int mipHeight = 512 * (unsigned int)std::pow(0.5f, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		PrefilterCubemapShader.SetUniform("uRoughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			PrefilterCubemapShader.SetUniform("uClipFromModel", uClipFromView * ViewFromModel[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, PrefilterMap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glBindVertexArray(RDI->PrimitivesCube->VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gConsole.LogInfo("TiledForwardRenderer::CaptureSpecularPrefilteredMap prefiltered cubemap captured");
}

void TiledForwardRenderer::CapturePreintegratedBRDF()
{
	// Create texture resource
	gConsole.LogInfo("TiledForwardRenderer::PreintegrateBRDF Create texture resource");
	glGenTextures(1, &PreintegratedBrdfLUT);
	glBindTexture(GL_TEXTURE_2D, PreintegratedBrdfLUT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	CHECK_GL_ERR();

	// Create FBO for BRDF preintegration
	gConsole.LogInfo("TiledForwardRenderer::PreintegrateBRDF Create capture FBO");
	GLuint captureFBO;
	GLuint captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	gConsole.LogInfo("TiledForwardRenderer::PreintegrateBRDF Capture FBO created");

	// Render preintegrated BRDF
	gConsole.LogInfo("TiledForwardRenderer::PreintegrateBRDF Render preintegrated BRDF");
	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	IntegrateBRDFShader.BindProgram();
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PreintegratedBrdfLUT, 0);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(RDI->PrimitivesQuad->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	gConsole.LogInfo("TiledForwardRenderer::CaptureCubemap preintegrated BRDF rendered");
}

void TiledForwardRenderer::CreateLightBuffers(const ScreenSize& size)
{
	// Define work group sizes in x and y direction based off screen size and tile size (in pixels)

	WorkGroupsX = (size.Width + (size.Width % 16)) / 16;
	WorkGroupsY = (size.Height + (size.Height % 16)) / 16;
	size_t numberOfTiles = WorkGroupsX * WorkGroupsY;
	gConsole.LogInfo("TiledForwardRenderer::Init workGroups: ({},{}), numberOfTiles: {}", WorkGroupsX, WorkGroupsY, numberOfTiles);

	// Generate our shader storage buffers
	glGenBuffers(1, &LightBuffer);
	glGenBuffers(1, &VisibleLightIndicesBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, LightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Light) * MAX_NUM_LIGHTS, 0, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, VisibleLightIndicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VisibleIndex) * MAX_NUM_LIGHTS * numberOfTiles, 0, GL_STATIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	CHECK_GL_ERR();
}

void TiledForwardRenderer::DeleteLightBuffers()
{
	if(LightBuffer > 0)
		glDeleteBuffers(1, &LightBuffer);

	if(VisibleLightIndicesBuffer > 0)
		glDeleteBuffers(1, &VisibleLightIndicesBuffer);
}

void TiledForwardRenderer::CreateRenderTargets(const ScreenSize& size)
{
	GLuint screenSizeX = size.Width;
	GLuint screenSizeY = size.Height;

	glGenFramebuffers(1, &FBOdepthMap);
	glGenTextures(1, &PreDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, PreDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenSizeX, screenSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, FBOdepthMap);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, PreDepthBuffer, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_ERR();
	CHECK_FBO_STATUS();

	// Create a floating point HDR frame buffer and a floating point color buffer (as a texture)
	glGenFramebuffers(1, &FBOhdr);

	glGenTextures(1, &ColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenSizeX, screenSizeY, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &NormalBuffer);
	glBindTexture(GL_TEXTURE_2D, NormalBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenSizeX, screenSizeY, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// It will also need a depth component as a render buffer, attached to the hdrFBO
	glGenRenderbuffers(1, &RboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, RboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenSizeX, screenSizeY);

	glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, NormalBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RboDepth);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, buffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_ERR();
	CHECK_FBO_STATUS();

	// Create pair of frame buffers for post process to swap
	glGenFramebuffers(1, &FBOpost0);

	glGenTextures(1, &PostColorBuffer0);
	glBindTexture(GL_TEXTURE_2D, PostColorBuffer0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenSizeX, screenSizeY, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, FBOpost0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PostColorBuffer0, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_ERR();
	CHECK_FBO_STATUS();

	glGenFramebuffers(1, &FBOpost1);

	glGenTextures(1, &PostColorBuffer1);
	glBindTexture(GL_TEXTURE_2D, PostColorBuffer1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenSizeX, screenSizeY, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, FBOpost1);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PostColorBuffer1, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(1, &LinearDepth);
	glBindTexture(GL_TEXTURE_2D, LinearDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenSizeX, screenSizeY, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	CHECK_GL_ERR();
	CHECK_FBO_STATUS();
}

void TiledForwardRenderer::Resize(const ScreenSize& size)
{
	gConsole.LogInfo("TiledForwardRenderer::Resize ({}, {})", size.Width, size.Height);

	DeleteLightBuffers();

	DeleteRenderTargets();

	CreateLightBuffers(size);

	CreateRenderTargets(size);
}

void TiledForwardRenderer::Deinit()
{
	gConsole.LogInfo("TiledForwardRenderer::Deinit");

	DeleteLightBuffers();

	DeleteRenderTargets();
}

void TiledForwardRenderer::DeleteRenderTargets()
{
	if (PreDepthBuffer > 0)
		glDeleteTextures(1, &PreDepthBuffer);

	if (FBOdepthMap>0)
		glDeleteFramebuffers(1, &FBOdepthMap);

	if (ColorBuffer > 0)
		glDeleteTextures(1, &ColorBuffer);

	if (NormalBuffer > 0)
		glDeleteTextures(1, &NormalBuffer);

	if (RboDepth > 0)
		glDeleteRenderbuffers(1, &RboDepth);

	if (FBOhdr > 0)
		glDeleteFramebuffers(1, &FBOhdr);

	if (PostColorBuffer0 > 0)
		glDeleteTextures(1, &PostColorBuffer0);

	if (FBOpost0 > 0)
		glDeleteFramebuffers(1, &FBOpost0);

	if (PostColorBuffer1 > 0)
		glDeleteTextures(1, &PostColorBuffer1);

	if (LinearDepth > 0)
		glDeleteTextures(1, &LinearDepth);

	if (FBOpost1 > 0)
		glDeleteFramebuffers(1, &FBOpost1);

	CHECK_GL_ERR();
	CHECK_FBO_STATUS();
}

void TiledForwardRenderer::Render(const SceneView& sceneView)
{
	// gConsole.LogInfo("TiledForwardRenderer::Render");

	UpdateLightsBufferFromScene(sceneView);

	RenderDepthPrePass(sceneView);

	ComputeLightCulling(sceneView);

	RenderOpaqueLit(sceneView);

	RenderSkybox(sceneView);

	// RenderEquiCube(sceneView);

	// RenderTranslucentLit(sceneView);

	// RenderParticleUnlit(sceneView.world, sceneView.cameraCmp);

	LinearizeDepth(sceneView);

	PostTonemapper(sceneView);

	// PostSSAO(cameraCmp);

	EditorDebug(sceneView);

	UIText2D(sceneView);

	PostGamma();
}

void TiledForwardRenderer::RenderEquiCube(const SceneView& sceneView)
{
	const Matrix clipFromWorld = sceneView.CameraCmp->GetClipFromWorld();

	EquiToCubemapShader.BindProgram();
	EquiToCubemapShader.SetUniform("uClipFromModel", clipFromWorld);

	// GLuint CubemapID = static_cast<const GLCubemapDeviceProxy*>(SkyboxWorldCmp->GetCubemap().GetTextureProxy())->GetTextureID();

	glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);

	glBindFragDataLocation(EquiToCubemapShader.GetProgramHandle(), (GLuint)0, "color");
	glBindFragDataLocation(EquiToCubemapShader.GetProgramHandle(), (GLuint)1, "normal");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, HDRTexture);
	EquiToCubemapShader.SetUniform("uEquirectangularMap", 0);

	// glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);

	glBindVertexArray(RDI->PrimitivesCube->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	// glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::RenderDepthPrePass(const SceneView& sceneView)
{
	DepthShader.BindProgram();

	glBindFramebuffer(GL_FRAMEBUFFER, FBOdepthMap);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	const Matrix& clipFromWorld = sceneView.CameraCmp->GetClipFromWorld();
	
	for (const MeshRenderingComponent* meshCmp : sceneView.OpaqueQueue)
	{
		const EntityTransform& transform = meshCmp->GetTransform();
		const Matrix& worldFromModel = transform.GetWorldFromModel();
		DepthShader.SetUniform("uScreenFromModel", clipFromWorld * worldFromModel);
		
		for (const MeshResource::SubMesh* subMesh : meshCmp->GetMesh()->GetSubMeshes())
		{
			const GLMeshDeviceProxy* meshProxy = static_cast<const GLMeshDeviceProxy*>(subMesh->GetMeshProxy());
			glBindVertexArray(meshProxy->GetVAO());
		
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, RDI->FallbackWhiteTexture);
		
			glDrawElements(GL_TRIANGLES, (GLsizei)subMesh->GetMeshData().GetTriangleCount() * 3, GL_UNSIGNED_INT, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::ComputeLightCulling(const SceneView& sceneView)
{
	float tme = (float)(sceneView.World->GetWorldComponent<TimeWorldComponent>()->GetGameplayTime());

	// gConsole.LogInfo("TiledForwardRenderer::LightCulling Time: {}, workGroupsX: {}, workGroupsY: {}",
	// 	Time, workGroupsX, workGroupsY
	// );

	LightCullingShader.BindProgram();
	LightCullingShader.SetUniform("uViewFromWorld", sceneView.CameraCmp->GetViewFromWorld());
	LightCullingShader.SetUniform("uClipFromWorld", sceneView.CameraCmp->GetClipFromWorld());
	LightCullingShader.SetUniform("uClipFromView",  sceneView.CameraCmp->GetClipFromView());
	LightCullingShader.SetUniform("uScreenSizeX", RDI->GetScreenSize().Width);
	LightCullingShader.SetUniform("uScreenSizeY", RDI->GetScreenSize().Height);
	LightCullingShader.SetUniform("uWorkGroupsX", (int)WorkGroupsX);
	LightCullingShader.SetUniform("uWorkGroupsY", (int)WorkGroupsY);
	LightCullingShader.SetUniform("uLightCount", (int)std::min(sceneView.PointLights.GetSize(), MAX_NUM_LIGHTS));

	// Bind depth map texture to texture location 4 (which will not be used by any model texture)
	glActiveTexture(GL_TEXTURE4);
	glUniform1i(glGetUniformLocation(LightCullingShader.GetProgramHandle(), "uDepthMap"), 4);
	glBindTexture(GL_TEXTURE_2D, PreDepthBuffer);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, LightBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VisibleLightIndicesBuffer);

	glDispatchCompute(WorkGroupsX, WorkGroupsY, 1);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TiledForwardRenderer::RenderOpaqueLit(const SceneView& sceneView)
{
	// gConsole.LogInfo("TiledForwardRenderer::AccumulateLights");

	glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	const EntityTransform& cameraTransform = sceneView.CameraCmp->GetTransform();

	LightAccumulationShader.BindProgram();
	LightAccumulationShader.SetUniform("uLightCount", (int)std::min(sceneView.PointLights.GetSize(), MAX_NUM_LIGHTS));
	LightAccumulationShader.SetUniform("uWorkGroupsX", (int)WorkGroupsX);
	LightAccumulationShader.SetUniform("uWorkGroupsY", (int)WorkGroupsY);
	LightAccumulationShader.SetUniform("uViewPosition", cameraTransform.GetGlobalTranslation());

	glBindFragDataLocation(LightAccumulationShader.GetProgramHandle(), 0, "oColor");
	glBindFragDataLocation(LightAccumulationShader.GetProgramHandle(), 1, "oNormal");

	int dirLightsCount = 0;
	for ( const DirectionalLightComponent* dirLightCmp : sceneView.DirectionalLights)
	{		
		const EntityTransform& transform = dirLightCmp->GetTransform();
		String baseName = String("uDirectionalLight[") + String::From(dirLightsCount) + String("].");
		Color colorIntensity = dirLightCmp->GetColor();
		colorIntensity.A = dirLightCmp->GetIntensity();
		LightAccumulationShader.SetUniform(baseName + "ColorIntensity", colorIntensity);
		LightAccumulationShader.SetUniform(baseName + "Direction", MovementSystem::GetGlobalForward(transform));

		++dirLightsCount;
		if (dirLightsCount == MAX_LIGHT_COUNT_DIRECTIONAL)
			break;
	}
	LightAccumulationShader.SetUniform("uDirectionalLightCount", dirLightsCount);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, LightBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VisibleLightIndicesBuffer);

	const Matrix& clipFromWorld = sceneView.CameraCmp->GetClipFromWorld();
	for (const MeshRenderingComponent* meshCmp : sceneView.OpaqueQueue)
	{
		const EntityTransform& transform = meshCmp->GetTransform();
		const Matrix& worldFromModel = transform.GetWorldFromModel();
		LightAccumulationShader.SetUniform("uClipFromModel", clipFromWorld * worldFromModel);
		LightAccumulationShader.SetUniform("uWorldFromModel", worldFromModel);
		
		int i = 0;
		for (const MeshResource::SubMesh* subMesh : meshCmp->GetMesh()->GetSubMeshes())
		{
			if (meshCmp->GetShadingModel() == eShadingModel::PBR)
			{
				// PhongMaterial material = meshCmp->GetMaterial(i);
				// lightAccumulationShader.SetUniform("uMaterial.Ambient", material.AmbientColor);
				// lightAccumulationShader.SetUniform("uMaterial.Diffuse", material.DiffuseColor);
				// lightAccumulationShader.SetUniform("uMaterial.Specular", material.SpecularColor);
				// lightAccumulationShader.SetUniform("uMaterial.Shininess", material.Shininess);

				PBRMaterial material = meshCmp->GetPBRMaterial(i);
				LightAccumulationShader.SetUniform("uMaterial.Emissive", material.Emissive);
				LightAccumulationShader.SetUniform("uMaterial.Albedo", material.Albedo);
				LightAccumulationShader.SetUniform("uMaterial.Roughness", material.Roughness);
				LightAccumulationShader.SetUniform("uMaterial.Metallic", material.Metallic);

				const GLMeshDeviceProxy* meshProxy = static_cast<const GLMeshDeviceProxy*>(subMesh->GetMeshProxy());
				glBindVertexArray(meshProxy->GetVAO());

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, IrradianceMap);
				LightAccumulationShader.SetUniform("uIrradianceMap", 0);

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_CUBE_MAP, PrefilterMap);
				LightAccumulationShader.SetUniform("uPrefilterMap", 1);

				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, PreintegratedBrdfLUT);
				LightAccumulationShader.SetUniform("uBrdfLUT", 2);

				const TextureResource* emissiveTexture = subMesh->GetMeshData().GetEmissiveMap();
				GLuint emissiveID = emissiveTexture == nullptr
					? RDI->FallbackBlackTexture
					: static_cast<const GLTextureDeviceProxy*>(emissiveTexture->GetTextureProxy())->GetTextureID();
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, emissiveID);
				LightAccumulationShader.SetUniform("uEmissiveMap", 3);

				const TextureResource* albedoTexture = subMesh->GetMeshData().GetAlbedoMap();
				GLuint albedoID = albedoTexture == nullptr
					? RDI->FallbackWhiteTexture
					: static_cast<const GLTextureDeviceProxy*>(albedoTexture->GetTextureProxy())->GetTextureID();
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, albedoID);
				LightAccumulationShader.SetUniform("uAlbedoMap", 4);
				
				const TextureResource* normalMap = subMesh->GetMeshData().GetNormalMap();
				GLuint normalMapID = normalMap == nullptr
					? RDI->FallbackNormalMap
					: static_cast<const GLTextureDeviceProxy*>(normalMap->GetTextureProxy())->GetTextureID();
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, normalMapID);
				LightAccumulationShader.SetUniform("uNormalMap", 5);
				 
				const TextureResource* roughnessTexture = subMesh->GetMeshData().GetRoughnessMap();
				GLuint roughnessID = roughnessTexture == nullptr
					? RDI->FallbackBlackTexture
					: static_cast<const GLTextureDeviceProxy*>(roughnessTexture->GetTextureProxy())->GetTextureID();
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, roughnessID);
				LightAccumulationShader.SetUniform("uRoughnessMap", 6);
				
				const TextureResource* metallicTexture = subMesh->GetMeshData().GetMetallicMap();
				GLuint metallicID = metallicTexture == nullptr
					? RDI->FallbackBlackTexture
					: static_cast<const GLTextureDeviceProxy*>(metallicTexture->GetTextureProxy())->GetTextureID();
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, metallicID);
				LightAccumulationShader.SetUniform("uMetallicMap", 7);
				
				const TextureResource* ambientOcclusionTexture = subMesh->GetMeshData().GetAmbientOcclusionMap();
				GLuint ambientOcclusionID = ambientOcclusionTexture == nullptr
					? RDI->FallbackWhiteTexture
					: static_cast<const GLTextureDeviceProxy*>(ambientOcclusionTexture->GetTextureProxy())->GetTextureID();
				glActiveTexture(GL_TEXTURE8);
				glBindTexture(GL_TEXTURE_2D, ambientOcclusionID);
				LightAccumulationShader.SetUniform("uAmbientOcclusionMap", 8);

				glDrawElements(GL_TRIANGLES, (GLsizei)subMesh->GetMeshData().GetTriangleCount() * 3, GL_UNSIGNED_INT, NULL);
				++i;
			}
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::RenderSkybox(const SceneView& sceneView)
{
	float time = (float)(sceneView.World->GetWorldComponent<TimeWorldComponent>()->GetGameplayTime());

	// const SkyboxWorldComponent* skyboxWorldCmp = world->GetWorldComponent<SkyboxWorldComponent>();
	// if (skyboxWorldCmp != nullptr)
	// {
		const Matrix clipFromView = sceneView.CameraCmp->GetClipFromView();
		Matrix viewFromWorld = sceneView.CameraCmp->GetViewFromWorld();
		// center cube in view space by setting translation to 0 for x, y and z.
		// SetTranslation resets Matrix to identity
		viewFromWorld.Data[3] = 0.0f;
		viewFromWorld.Data[7] = 0.0f;
		viewFromWorld.Data[11] = 0.0f;

		Matrix clipFromWorld = clipFromView * viewFromWorld;

		SkyboxShader.BindProgram();
		SkyboxShader.SetUniform("uClipFromWorld", clipFromWorld);
		// SkyboxShader.SetUniform("uTime", Time);

		// GLuint CubemapID = static_cast<const GLCubemapDeviceProxy*>(SkyboxWorldCmp->GetCubemap().GetTextureProxy())->GetTextureID();

		glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);
		
		glBindFragDataLocation(LightAccumulationShader.GetProgramHandle(), 0, "color");
		glBindFragDataLocation(LightAccumulationShader.GetProgramHandle(), 1, "normal");
		
		glDisable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL);

		glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_CUBE_MAP, CubemapID);
		// glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		// glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemap);

		glBindVertexArray(RDI->PrimitivesCube->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// }
}

void TiledForwardRenderer::RenderTranslucentLit(const SceneView& sceneView)
{
	// gConsole.LogInfo("TiledForwardRenderer::RenderTranslucenLit");
	
	glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	// glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const EntityTransform& cameraTransform = sceneView.CameraCmp->GetTransform();
	TranslucentShader.BindProgram();
	TranslucentShader.SetUniform("uViewPosition", cameraTransform.GetGlobalTranslation());

	int dirLightsCount = 0;
	for (const DirectionalLightComponent* dirLightCmp: sceneView.DirectionalLights)
	{
		const EntityTransform& transform = dirLightCmp->GetTransform();
		String baseName = String("uDirectionalLight[") + String::From(dirLightsCount) + String("].");
		Color colorIntensity = dirLightCmp->GetColor();
		colorIntensity.A = dirLightCmp->GetIntensity();
		TranslucentShader.SetUniform(baseName + "ColorIntensity", colorIntensity);
		TranslucentShader.SetUniform(baseName + "Direction", MovementSystem::GetGlobalForward(transform));

		++dirLightsCount;
		if (dirLightsCount == MAX_LIGHT_COUNT_DIRECTIONAL)
			break;
	}
	TranslucentShader.SetUniform("uDirectionalLightCount", dirLightsCount);

	glBindFragDataLocation(TranslucentShader.GetProgramHandle(), 0, "color");
	glBindFragDataLocation(TranslucentShader.GetProgramHandle(), 1, "normal");
	// glBindFragDataLocation(TranslucentShader.GetProgramHandle(), 0, "oColor");
	// glBindFragDataLocation(TranslucentShader.GetProgramHandle(), 1, "oNormal");

	const Matrix& clipFromWorld = sceneView.CameraCmp->GetClipFromWorld();
	for (const MeshRenderingComponent* meshCmp : sceneView.TranslucentQueue)
	{
		const EntityTransform& transform = meshCmp->GetTransform();
		const Matrix& worldFromModel = transform.GetWorldFromModel();
		TranslucentShader.SetUniform("uClipFromModel", clipFromWorld * worldFromModel);
		TranslucentShader.SetUniform("uWorldFromModel", worldFromModel);

		int i = 0;
		for (const MeshResource::SubMesh* subMesh : meshCmp->GetMesh()->GetSubMeshes())
		{
			PBRMaterial material = meshCmp->GetPBRMaterial(i);
			TranslucentShader.SetUniform("uMaterial.Ambient", Color::BLACK);
			TranslucentShader.SetUniform("uMaterial.Diffuse", material.Albedo);
			TranslucentShader.SetUniform("uMaterial.Specular", material.Albedo);
			TranslucentShader.SetUniform("uMaterial.Shininess", 16.0f);

			const GLMeshDeviceProxy* meshProxy = static_cast<const GLMeshDeviceProxy*>(subMesh->GetMeshProxy());
			glBindVertexArray(meshProxy->GetVAO());

			const TextureResource* diffuseTexture = subMesh->GetMeshData().GetAlbedoMap();
			GLuint diffuseID = diffuseTexture == nullptr
				? RDI->FallbackWhiteTexture
				: static_cast<const GLTextureDeviceProxy*>(diffuseTexture->GetTextureProxy())->GetTextureID();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseID);
			TranslucentShader.SetUniform("uAlbedoMap", 0);

			const TextureResource* normalMap = subMesh->GetMeshData().GetNormalMap();
			GLuint normalMapID = normalMap == nullptr
				? RDI->FallbackNormalMap
				: static_cast<const GLTextureDeviceProxy*>(normalMap->GetTextureProxy())->GetTextureID();
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalMapID);
			TranslucentShader.SetUniform("uNormalMap", 1);

			const TextureResource* specularTexture = subMesh->GetMeshData().GetRoughnessMap();
			GLuint specularID = specularTexture == nullptr
				? RDI->FallbackWhiteTexture
				: static_cast<const GLTextureDeviceProxy*>(specularTexture->GetTextureProxy())->GetTextureID();
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, specularID);
			TranslucentShader.SetUniform("uSpecularMap", 2);

			glDrawElements(GL_TRIANGLES, (GLsizei)subMesh->GetMeshData().GetTriangleCount() * 3, GL_UNSIGNED_INT, NULL);
			++i;
		}
	}
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);

	// glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::RenderParticleUnlit(World* world, const CameraComponent* cameraCmp)
{
	// gConsole.LogInfo("TiledForwardRenderer::RenderParticleUnlit");

	float time = (float)TimeSystem::GetTimerElapsedTime(world, eEngineTimer::GAMEPLAY);
	const Matrix& viewFromWorld = cameraCmp->GetViewFromWorld();
	const Matrix& screenFromView = cameraCmp->GetClipFromView();

	glBindFramebuffer(GL_FRAMEBUFFER, FBOhdr);

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	// TODO test these blending options
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
	//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);

	ParticleShader.BindProgram();
	ParticleShader.SetUniform("uTime", time);
	ParticleShader.SetUniform("uScreenFromView", screenFromView);

	glBindFragDataLocation(TranslucentShader.GetProgramHandle(), 0, "color");
	glBindFragDataLocation(TranslucentShader.GetProgramHandle(), 1, "normal");
	// glBindFragDataLocation(TranslucentShader.GetProgramHandle(), 0, "oColor");
	// glBindFragDataLocation(TranslucentShader.GetProgramHandle(), 1, "oNormal");

	for (auto componentsTuple : world->IterateComponents<ParticleComponent>())
	{
		const ParticleComponent* particleCmp = std::get<ParticleComponent*>(componentsTuple);
		const EntityTransform& transform = particleCmp->GetTransform();
		const Matrix& worldFromModel = particleCmp->GetEmitter()->GetSettings().SimulationSpace == ParticleEmitter::eSimulationSpace::LOCAL_SPACE
			? transform.GetWorldFromModel()
			: Matrix();

		ParticleShader.SetUniform("uViewFromWorld", viewFromWorld);
		ParticleShader.SetUniform("uWorldFromModel", worldFromModel);

		ParticleEmitter::Settings emitterSettings = particleCmp->GetEmitter()->GetSettings();
		ParticleShader.SetUniform("uEmitterColor", emitterSettings.BaseColor);

		SpritesheetSettings spriteSettings = emitterSettings.SpritesheetSettings;
		ParticleShader.SetUniform("uSpriteColor", spriteSettings.SpriteColor);
		float startFrame = spriteSettings.IsRandomStartFrame ? RandomRange(0.0f, spriteSettings.SubImages.X * spriteSettings.SubImages.Y) : spriteSettings.StartFrame;
		ParticleShader.SetUniform("uSpriteStartFrame", startFrame);
		ParticleShader.SetUniform("uSpriteSpeed", spriteSettings.Speed);
		ParticleShader.SetUniform("uSpriteSubImages", spriteSettings.SubImages.X, spriteSettings.SubImages.Y);

		GLsizei partileLen = (GLsizei)(particleCmp->GetEmitter()->GetInstancesCount());
		const TextureResource* texture = particleCmp->GetEmitter()->GetSpritesheet();
		const GLParticleDeviceProxy* particleProxy = static_cast<const GLParticleDeviceProxy*>(particleCmp->GetEmitter()->GetParticleProxy());
		GLuint particleVAO = particleProxy->GetVAO();

		GLuint textureID = texture == nullptr
			? RDI->FallbackWhiteTexture
			: static_cast<const GLTextureDeviceProxy*>(texture->GetTextureProxy())->GetTextureID();

		ParticleShader.SetUniform("uHasSprite", texture == nullptr ? 0.0f : 1.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glBindVertexArray(particleVAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, partileLen);
		glBindVertexArray(0);
		CHECK_GL_ERR();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::LinearizeDepth(const SceneView& sceneView)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Weirdly, moving this call drops performance into the floor

	glBindFramebuffer(GL_FRAMEBUFFER, FBOpost1);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, LinearDepth, 0);

	LinearizeDepthShader.BindProgram();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, PreDepthBuffer);
	LinearizeDepthShader.SetUniform("uDepth", 0);
	LinearizeDepthShader.SetUniform("uNear", sceneView.CameraCmp->GetClippingPlaneNear());
	LinearizeDepthShader.SetUniform("uFar", sceneView.CameraCmp->GetClippingPlaneFar());

	glBindVertexArray(RDI->PrimitivesQuad->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, PostColorBuffer1, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::PostTonemapper(const SceneView& sceneView)
{
	// gConsole.LogInfo("TiledForwardRenderer::Tonemapper");

	float exposure = 1.0f;
	const PostprocessSettingsComponent* postCmp = sceneView.CameraCmp->GetSibling<PostprocessSettingsComponent>();
	if (postCmp) {
		exposure = postCmp->Exposure;
	}

	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Weirdly, moving this call drops performance into the floor
	
	glBindFramebuffer(GL_FRAMEBUFFER, FBOpost0);	

	HDRShader.BindProgram();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ColorBuffer);
	HDRShader.SetUniform("uExposure", exposure);

	glBindVertexArray(RDI->PrimitivesQuad->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::PostSSAO(const SceneView& sceneView)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBOpost1);

	// based on: http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
	static Dynarray<Vector> ssaoKernel;
	if (ssaoKernel.GetSize() <= 0) {
		gConsole.LogInfo("TiledForwardRenderer::PostSSAO generating kernel");
		for (int i = 0; i < 16; ++i) {
			ssaoKernel.PushBack(Vector(
				RandomRange(-1.0f, 1.0f),
				RandomRange(-1.0f, 1.0f),
				RandomRange( 0.0f, 1.0f)
			));
			ssaoKernel[i].Normalize();
			ssaoKernel[i] *= RandomRange(0.0f, 1.0f);
			float scale = float(i) / 16.0;
			scale = Lerp(0.1f, 1.0f, scale * scale);
			ssaoKernel[i] *= scale;
			gConsole.LogInfo("TiledForwardRenderer::PostSSAO kernel #{} {}", i, ssaoKernel[i]);
		}
	}

	SSAOShader.BindProgram();
	for (int i = 0; i < 16; ++i) {
		SSAOShader.SetUniform(String("uKernel[") + String::From(i) + "]", ssaoKernel[i]);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, PostColorBuffer0);
	SSAOShader.SetUniform("uBeauty", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalBuffer);
	SSAOShader.SetUniform("uNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, PreDepthBuffer);
	SSAOShader.SetUniform("uDepth", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, RDI->SSAONoiseMap);
	SSAOShader.SetUniform("uNoise", 3);
	SSAOShader.SetUniform("uScreenRes", Vector(
		RDI->GetScreenSize().Width,
		RDI->GetScreenSize().Height,
		1.0f / ((float)RDI->GetScreenSize().Width),
		1.0f / ((float)RDI->GetScreenSize().Height))
	);
	SSAOShader.SetUniform("uFarClippingPlane", sceneView.CameraCmp->GetClippingPlaneFar());
	SSAOShader.SetUniform("uBias", 0.3f);
	SSAOShader.SetUniform("uViewFromWorld", sceneView.CameraCmp->GetViewFromWorld());

	glBindVertexArray(RDI->PrimitivesQuad->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::EditorDebug(const SceneView& sceneView)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBOpost0);

	const Matrix& clipFromWorld = sceneView.CameraCmp->GetClipFromWorld();
	EditorDebugShader.BindProgram();
	EditorDebugShader.SetUniform("uMVP", clipFromWorld);

	// Render Lines
	{
		auto debugLinesComponent = sceneView.World->GetWorldComponent<DebugDrawStateWorldComponent>();
		auto& debugLines = debugLinesComponent->DebugLines;
		auto& debugLinesColors = debugLinesComponent->DebugLinesColors;

		DebugRenderingBuffers debugLinesBuffers; // vertex and color buffer

												 // set up buffer
		glBindVertexArray(debugLinesBuffers.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, debugLinesBuffers.VBO);
		glBufferData(GL_ARRAY_BUFFER, debugLines.GetSize() * sizeof(DebugDrawStateWorldComponent::DebugLine)
			+ debugLinesColors.GetSize() * sizeof(DebugDrawStateWorldComponent::DebugLineColor), NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, debugLines.GetSize() * sizeof(DebugDrawStateWorldComponent::DebugLine), (GLvoid*)debugLines.GetData());
		glBufferSubData(GL_ARRAY_BUFFER, debugLines.GetSize() * sizeof(DebugDrawStateWorldComponent::DebugLine), debugLinesColors.GetSize() * sizeof(DebugDrawStateWorldComponent::DebugLineColor), (GLvoid*)debugLinesColors.GetData());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), NULL);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Color), (GLvoid*)(debugLines.GetSize() * sizeof(DebugDrawStateWorldComponent::DebugLine)));
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_LINES, 0, (GLsizei)debugLines.GetSize() * 2);
		glBindVertexArray(0);

		debugLines.Clear();
		debugLinesColors.Clear();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::UIText2D(const SceneView& sceneView)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBOpost0);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ScreenSize screen = RDI->GetScreenSize();
	Matrix ortho;
	ortho.SetOrthographic(
		sceneView.Rect.GetMin().Y * screen.Height,
		sceneView.Rect.GetMax().Y * screen.Height,
		sceneView.Rect.GetMin().X * screen.Width,
		sceneView.Rect.GetMax().X * screen.Width,
		-1,
		 1
	);
	Text2DShader.BindProgram();
	Text2DShader.SetUniform("u_projection", ortho);

	for (auto componentsTuple : sceneView.World->IterateComponents<ScreenSpaceTextComponent>())
	{
		ScreenSpaceTextComponent* textCmp = std::get<ScreenSpaceTextComponent*>(componentsTuple);
		Text2D& text = textCmp->GetText();
		Text2DShader.SetUniform("u_textColor", text.GetFontColor());
		Text2DShader.SetUniform("u_position", Vector((float)textCmp->GetScreenPosition().X, (float)textCmp->GetScreenPosition().Y, 0));
		text.UpdateDeviceBuffers();

		const GLTextFieldBufferDeviceProxy* textFieldBuffer = static_cast<const GLTextFieldBufferDeviceProxy*>(text.GetTextFieldBuffer());

		glBindVertexArray(textFieldBuffer->GetVAO());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, static_cast<const GLTextureDeviceProxy*>(text.GetFontTextureProxy())->GetTextureID());

		// Render glyph texture over quad
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(6 * textFieldBuffer->GetSize()));

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	CHECK_GL_ERR();

	glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TiledForwardRenderer::PostGamma()
{
	GammaShader.BindProgram();
	glActiveTexture(GL_TEXTURE0);	
	glBindTexture(GL_TEXTURE_2D, PostColorBuffer0);
	GammaShader.SetUniform("uGamma", 2.2f);

	glBindVertexArray(RDI->PrimitivesQuad->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void TiledForwardRenderer::SetupLightsBufferFromScene()
{
	if (LightBuffer == 0)
	{
		return;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, LightBuffer);
	Light *lights = (Light*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	for (int i = 0; i < MAX_NUM_LIGHTS; ++i)
	{
		Light &light = lights[i];
		light.Position = Vector::ZERO;
		light.Color = Vector::ZERO;
		light.RangeIntensity = Vector(-1.0f, 0.0f, 0.0f);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void TiledForwardRenderer::UpdateLightsBufferFromScene(const SceneView& sceneView)
{
	Dynarray<Vector> positions;
	Dynarray<Vector> color;
	Dynarray<Vector> rangeIntensity;

	int lightCounter = 0;
	for (const PointLightComponent* pointLightCmp : sceneView.PointLights)
	{
		const EntityTransform& transform = pointLightCmp->GetTransform();

		positions.PushBack(transform.GetGlobalTranslation());
		color.PushBack(Vector(pointLightCmp->GetColor()));
		rangeIntensity.PushBack(Vector(pointLightCmp->GetRange(), pointLightCmp->GetIntensity(), 0.0f));

		++lightCounter;

		if (lightCounter > MAX_NUM_LIGHTS)
		{
			gConsole.LogInfo("TiledForwardRenderer::UpdateLightsBufferFromScene more lights than supported by renderer({})", MAX_NUM_LIGHTS);
			break;
		}
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, LightBuffer);
	Light *lights = (Light*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	for (int i = 0; i < lightCounter; ++i)
	{
		Light &light = lights[i];
		light.Position = positions[i];
		light.Color = color[i];
		light.RangeIntensity = rangeIntensity[i];

		// 		gConsole.LogInfo("TiledForwardRenderer::UpdateLightsBufferFromScene set Position: {}, Radius: {}",
		// 			light.Position, light.Radius);
	}

	int remainingSlots = MAX_NUM_LIGHTS - lightCounter;
	for (int i = 0; i < remainingSlots; ++i)
	{
		Light &light = lights[lightCounter + i];
		light.Position = Vector::ZERO;
		light.Color = Vector::ZERO;
		light.RangeIntensity = Vector(-1.0f, 0.0f, 0.0f);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void TiledForwardRenderer::DebugDepthPrepass(const SceneView& sceneView)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Weirdly, moving this call drops performance into the floor
	DebugQuadDepthPrepassShader.BindProgram();
	DebugQuadDepthPrepassShader.SetUniform("uNear", sceneView.CameraCmp->GetClippingPlaneNear());
	DebugQuadDepthPrepassShader.SetUniform("uFar", sceneView.CameraCmp->GetClippingPlaneFar());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, PreDepthBuffer);

	glBindVertexArray(RDI->PrimitivesQuad->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void TiledForwardRenderer::DebugLightAccum(const SceneView& sceneView)
{
	float time = (float)(sceneView.World->GetWorldComponent<TimeWorldComponent>()->GetGameplayTime());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DebugLightAccumShader.BindProgram();
	DebugLightAccumShader.SetUniform("uTime", time);
	DebugLightAccumShader.SetUniform("uWorkGroupsX", (int)WorkGroupsX);
	DebugLightAccumShader.SetUniform("uWorkGroupsY", (int)WorkGroupsY);
	DebugLightAccumShader.SetUniform("uLightCount", (int)std::min(sceneView.PointLights.GetSize(), MAX_NUM_LIGHTS));

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, LightBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VisibleLightIndicesBuffer);

	const Matrix& clipFromWorld = sceneView.CameraCmp->GetClipFromWorld();
	for (auto componentsTuple : sceneView.World->IterateComponents<MeshRenderingComponent>())
	{
		const MeshRenderingComponent* meshCmp = std::get<MeshRenderingComponent*>(componentsTuple);
		const EntityTransform& transform = meshCmp->GetTransform();
		const Matrix& worldFromModel = transform.GetWorldFromModel();
		DebugLightAccumShader.SetUniform("uClipFromModel", clipFromWorld * worldFromModel);
		DebugLightAccumShader.SetUniform("uWorldFromModel", worldFromModel);

		for (const MeshResource::SubMesh* subMesh : meshCmp->GetMesh()->GetSubMeshes())
		{
			const GLMeshDeviceProxy* meshProxy = static_cast<const GLMeshDeviceProxy*>(subMesh->GetMeshProxy());
			glBindVertexArray(meshProxy->GetVAO());

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, RDI->FallbackWhiteTexture);

			glDrawElements(GL_TRIANGLES, (GLsizei)subMesh->GetMeshData().GetTriangleCount() * 3, GL_UNSIGNED_INT, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindVertexArray(0);
		}
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
}