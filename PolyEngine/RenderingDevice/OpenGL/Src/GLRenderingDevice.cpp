#include <PolyRenderingDeviceGLPCH.hpp>

#include <GLRenderingDevice.hpp>

#include <Common/GLUtils.hpp>
#include <Common/PrimitiveQuad.hpp>
#include <Common/PrimitiveCube.hpp>

#include <Proxy/GLTextureDeviceProxy.hpp>
#include <Proxy/GLCubemapDeviceProxy.hpp>
#include <Proxy/GLParticleDeviceProxy.hpp>
#include <Proxy/GLTextFieldBufferDeviceProxy.hpp>
#include <Proxy/GLMeshDeviceProxy.hpp>
#include <Proxy/GLShaderDeviceProxy.hpp>

#include <Pipeline/UnlitRenderingPass.hpp>
#include <Pipeline/BlinnPhongRenderingPass.hpp>
#include <Pipeline/Text2DRenderingPass.hpp>
#include <Pipeline/DebugNormalsRenderingPass.hpp>
#include <Pipeline/DebugNormalsWireframeRenderingPass.hpp>
#include <Pipeline/DebugRenderingPass.hpp>
#include <Pipeline/ParticlesRenderingPass.hpp>
#include <Pipeline/PostprocessRenderingPass.hpp>
#include <Pipeline/TransparentRenderingPass.hpp>
#include <Pipeline/SpritesheetRenderingPass.hpp>
#include <Pipeline/SkyboxRenderingPass.hpp>

#include <IRendererInterface.hpp>

using namespace Poly;

GLRenderingDevice* Poly::gRenderingDevice = nullptr;

IRenderingDevice* POLY_STDCALL PolyCreateRenderingDevice(SDL_Window* window, const Poly::ScreenSize& size) { return new GLRenderingDevice(window, size); }

GLRenderingDevice::GLRenderingDevice(SDL_Window* window, const Poly::ScreenSize& size)
: Window(window), ScreenDim(size)
{
	ASSERTE(window, "Invalid window passed to rendering device.");
	ASSERTE(gRenderingDevice == nullptr, "Creating device twice?");
	gRenderingDevice = this;
	
	if (!CreateContextHighend()) {
		ASSERTE(Context, "Context creation failed! OpenGL 4.3 is required! Sorry we probably do not support your GPU and Mac OS.");
	}

	ASSERTE(Context, "OpenGL context creation failed!");

	gConsole.LogInfo("OpenGL context set up successfully");
	gConsole.LogInfo("GL Renderer: {}", glGetString(GL_RENDERER));
	gConsole.LogInfo("GL Version: {}", glGetString(GL_VERSION));
	gConsole.LogInfo("GLSL Version: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));
	gConsole.LogInfo("Fitting renderer (0 - Fallback, 1 - Highend): {}", (int)RendererType);

	// Setup V-Sync
	SDL_GL_SetSwapInterval(1);
}

bool GLRenderingDevice::CreateContextHighend()
{
	bool isSuccessfull = false;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	Context = SDL_GL_CreateContext(Window);
	isSuccessfull = (bool)Context;

	if (Context)
	{
		RendererType = eRendererType::TILED_FORWARD;
		gConsole.LogInfo("HighEnd context created");
	}

	if (!Context)
	{
		gConsole.LogInfo("HighEnd context setup failed, err: {}", SDL_GetError());
	}

	return isSuccessfull;
}

GLRenderingDevice::~GLRenderingDevice()
{
	Renderer->Deinit();

	CleanUpResources();

	if(Context)
		SDL_GL_DeleteContext(Context);

	gRenderingDevice = nullptr;
}

void GLRenderingDevice::EndFrame()
{
	if(Window && Context)
		SDL_GL_SwapWindow(Window);
}

void GLRenderingDevice::Resize(const ScreenSize& size)
{
	ScreenDim = size;

	for (eRenderTargetId targetType : IterateEnum<eRenderTargetId>())
	{
		if (RenderingTargets[targetType])
		{
			RenderingTargets[targetType]->Resize(size);
		}
	}

	if (Renderer)
	{
		Renderer->Resize(size);
	}
}

void GLRenderingDevice::GetExtensions()
{
	int extensionsSize = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionsSize);
	OpenGLExtensions.Clear();
	OpenGLExtensions.Reserve(extensionsSize);

	gConsole.LogInfo("OpenGL supported extensions:");
	for (int i = 0; i<extensionsSize; i++)
	{
		String extension = String((const char*)glGetStringi(GL_EXTENSIONS, i));
		OpenGLExtensions.PushBack(extension);
		gConsole.LogInfo("Ext {}: {}", i, extension);
	}
}

template<typename T>
inline void GLRenderingDevice::RegisterGeometryPass(eGeometryRenderPassType type, const std::initializer_list<InputOutputBind>& inputs, const std::initializer_list<InputOutputBind>& outputs)
{
	RegisterGeometryPassWithArgs<T>(type, inputs, outputs);
}

template<typename T, class... Args_t>
inline void GLRenderingDevice::RegisterGeometryPassWithArgs(eGeometryRenderPassType type, const std::initializer_list<InputOutputBind>& inputs, const std::initializer_list<InputOutputBind>& outputs, Args_t&&... args)
{
	GeometryRenderingPasses[type] = std::make_unique<T>(this, std::forward<Args_t>(args)...);

	for (const InputOutputBind& bind : outputs)
		GeometryRenderingPasses[type]->BindOutput(bind.Name, bind.Target);

	for (const InputOutputBind& bind : inputs)
		GeometryRenderingPasses[type]->BindInput(bind.Name, bind.Target);

	GeometryRenderingPasses[type]->Finalize();
}

template<typename T, typename... Args>
T* Poly::GLRenderingDevice::CreateRenderingTarget(eRenderTargetId type, Args&&... args)
{
	T* target = new T(std::forward<Args>(args)...);
	RenderingTargets[type].reset(target);
	return target;
}

void Poly::GLRenderingDevice::RegisterPostprocessPass(ePostprocessRenderPassType type, const String& fragShaderName, const std::initializer_list<InputOutputBind>& inputs, const std::initializer_list<InputOutputBind>& outputs)
{
	PostprocessRenderingPasses[type] = std::make_unique<PostprocessRenderingPass>(this, fragShaderName);

	for (const InputOutputBind& bind : outputs)
		PostprocessRenderingPasses[type]->BindOutput(bind.Name, bind.Target);

	for (const InputOutputBind& bind : inputs)
		PostprocessRenderingPasses[type]->BindInput(bind.Name, bind.Target);

	PostprocessRenderingPasses[type]->Finalize();
}

void Poly::GLRenderingDevice::CleanUpResources()
{
	for (eRenderTargetId targetType : IterateEnum<eRenderTargetId>())
		RenderingTargets[targetType].reset();

	for (eGeometryRenderPassType passType : IterateEnum<eGeometryRenderPassType>())
		GeometryRenderingPasses[passType].reset();

	for (ePostprocessRenderPassType passType : IterateEnum<ePostprocessRenderPassType>())
		PostprocessRenderingPasses[passType].reset();

	PrimitivesQuad.reset();
	PrimitivesCube.reset();
}

std::unique_ptr<ITextureDeviceProxy> GLRenderingDevice::CreateTexture(size_t width, size_t height, size_t channels, eTextureUsageType usage)
{
	return std::make_unique<GLTextureDeviceProxy>(width, height, channels, usage);
}

std::unique_ptr<ICubemapDeviceProxy> Poly::GLRenderingDevice::CreateCubemap(size_t width, size_t height)
{
	return std::make_unique<GLCubemapDeviceProxy>(width, height);
}

std::unique_ptr<ITextFieldBufferDeviceProxy> GLRenderingDevice::CreateTextFieldBuffer()
{
	return std::make_unique<GLTextFieldBufferDeviceProxy>();
}

std::unique_ptr<IMeshDeviceProxy> GLRenderingDevice::CreateMesh()
{
	return std::make_unique<GLMeshDeviceProxy>();
}

std::unique_ptr<IParticleDeviceProxy> GLRenderingDevice::CreateParticle()
{
	return std::make_unique<GLParticleDeviceProxy>();
}

std::unique_ptr<IShaderDeviceProxy> GLRenderingDevice::CreateShader()
{
	return std::make_unique<GLShaderDeviceProxy>();
}

std::unique_ptr<GLShaderProgram> GLRenderingDevice::CreateShaderProgram()
{
	return std::make_unique<GLShaderProgram>(ShaderCompiler);
}