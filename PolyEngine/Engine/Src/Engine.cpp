#include <EnginePCH.hpp>

#include <Engine.hpp>
#include <Configs/AssetsPathConfig.hpp>
#include <Configs/DebugConfig.hpp>
#include <ECS/DeferredTaskSystem.hpp>
#include <Input/InputWorldComponent.hpp>
#include <Movement/FreeFloatMovementComponent.hpp>
#include <AI/PathfindingSystem.hpp>
#include <Rendering/ViewportWorldComponent.hpp>
#include <Rendering/Lighting/LightSourceComponent.hpp>
#include <Rendering/RenderingSystem.hpp>
#include <Rendering/Particles/ParticleUpdateSystem.hpp>
#include <Time/TimeWorldComponent.hpp>
#include <Imgui/ImguiSystem.hpp>
#include <Debugging/DebugWorldComponent.hpp>
#include <Debugging/DebugDrawComponents.hpp>
#include <Physics2D/Physics2DWorldComponent.hpp>
#include <Physics3D/Physics3DWorldComponent.hpp>
#include <Debugging/DebugDrawSystem.hpp>
#include <ECS/LambdaSystem.hpp>

using namespace Poly;

Engine* Poly::gEngine = nullptr;

//------------------------------------------------------------------------------
Engine::Engine(bool testRun, IEditor* editor)
	: Game(), Editor(editor)
{
	ASSERTE(gEngine == nullptr, "Creating engine twice?");
	gEngine = this;

	if (!testRun)
	{
		RandomSetSeed((int)time(nullptr));

		if (Editor)
			gAssetsPathConfig.DeserializeFromFile(Editor->GetAssetsPathConfigPath());
		else
			gAssetsPathConfig.Load();

		gDebugConfig.Load();
		// also set presets for debug draw (DebugDrawPresets)
		// @todo update debug draw presets from GUI
		gDebugConfig.DebugDrawPresets |= eDebugDrawPreset::GFX;
	}
}

//------------------------------------------------------------------------------
void Engine::Init(std::unique_ptr<IGame> game, std::unique_ptr<IRenderingDevice> device)
{
	Game = std::move(game);
	RenderingDevice = std::move(device);
	RenderingDevice->Init();

	// Engine update phases
	RegisterUpdatePhase(TimeSystem::TimeUpdatePhase, eUpdatePhaseOrder::PREUPDATE);
	RegisterUpdatePhase(InputSystem::InputPhase, eUpdatePhaseOrder::PREUPDATE);
	RegisterSystem(std::make_unique<ImguiSystem>(), eUpdatePhaseOrder::PREUPDATE);
	RegisterUpdatePhase(Physics2DSystem::Physics2DUpdatePhase, eUpdatePhaseOrder::PREUPDATE);
	RegisterUpdatePhase(Physics3DSystem::Physics3DUpdatePhase, eUpdatePhaseOrder::PREUPDATE);
	RegisterUpdatePhase(MovementSystem::MovementUpdatePhase, eUpdatePhaseOrder::PREUPDATE);

	RegisterUpdatePhase(PathfindingSystem::UpdatePhase, eUpdatePhaseOrder::POSTUPDATE);
	RegisterUpdatePhase(CameraSystem::CameraUpdatePhase, eUpdatePhaseOrder::POSTUPDATE);
	RegisterUpdatePhase(DebugDrawSystem::DebugRenderingUpdatePhase, eUpdatePhaseOrder::POSTUPDATE);
	RegisterUpdatePhase(ParticleUpdateSystem::ParticleUpdatePhase, eUpdatePhaseOrder::POSTUPDATE);
	RegisterUpdatePhase(RenderingSystem::RenderingPhase, eUpdatePhaseOrder::POSTUPDATE);
	RegisterUpdatePhase(SoundSystem::SoundPhase, eUpdatePhaseOrder::POSTUPDATE);
	RegisterUpdatePhase(DeferredTaskSystem::DeferredTaskPhase, eUpdatePhaseOrder::POSTUPDATE);
	RegisterUpdatePhase(FPSSystem::FPSUpdatePhase, eUpdatePhaseOrder::POSTUPDATE);

	LoadDefaultScene();

	Game->RegisterEngine(this);
}

//------------------------------------------------------------------------------
void Poly::Engine::StartGame()
{
	// Placeholder for real scene system
	LoadDefaultScene();

	SoundSystem::SetWorldCurrent(GetActiveScene());

	// Init game
	Game->Init();

	if (Editor)
		Editor->OnGameInit();
}

//------------------------------------------------------------------------------
void Poly::Engine::EndGame()
{
	if (Editor)
		Editor->OnGameDeinit();
	Game->Deinit();
}

//------------------------------------------------------------------------------
Engine::~Engine()
{
	EndGame();
	ActiveScene.reset();
	Game.reset();
	RenderingDevice.reset();
	gEngine = nullptr;
}

//------------------------------------------------------------------------------
void Engine::RegisterUpdatePhase(const PhaseUpdateFunction& phaseFunction, eUpdatePhaseOrder order)
{
	HEAVY_ASSERTE(order != eUpdatePhaseOrder::_COUNT, "_COUNT enum value passed to RegisterUpdatePhase(), which is an invalid value");
	Dynarray<std::unique_ptr<ISystem>>& UpdatePhases = GameUpdatePhases[order];
	UpdatePhases.PushBack(std::make_unique<LambdaSystem>(phaseFunction));
}

//------------------------------------------------------------------------------
void Engine::RegisterSystem(std::unique_ptr<ISystem> system, eUpdatePhaseOrder order)
{
	HEAVY_ASSERTE(order != eUpdatePhaseOrder::_COUNT, "_COUNT enum value passed to RegisterUpdatePhase(), which is an invalid value");
	Dynarray<std::unique_ptr<ISystem>>& UpdatePhases = GameUpdatePhases[order];
	UpdatePhases.PushBack(std::move(system));
}


//------------------------------------------------------------------------------
void Engine::Update()
{
	UpdatePhases(eUpdatePhaseOrder::PREUPDATE);
	UpdatePhases(eUpdatePhaseOrder::UPDATE);
	UpdatePhases(eUpdatePhaseOrder::POSTUPDATE);
}

void Engine::Update(Dynarray<eUpdatePhaseOrder> phasesToUpdate)
{
	for (auto phase : phasesToUpdate)
		UpdatePhases(phase);
}

//------------------------------------------------------------------------------
void Engine::ResizeScreen(const ScreenSize & size)
{
	gConsole.LogDebug("Screen resize: {} {}", size.Width, size.Height);
	GetRenderingDevice()->Resize(size);
}

void Engine::RequestGameQuit()
{
	QuitRequested = true;
}

void Poly::Engine::LoadDefaultScene()
{
	//@todo(muniu) implement entities staying across scenes.
	//@todo(muniu) implement loading custom scenes.
	ActiveScene = std::make_unique<Scene>();

	// Add WorldComponents
	DeferredTaskSystem::AddWorldComponentImmediate<InputWorldComponent>(GetActiveScene());
	DeferredTaskSystem::AddWorldComponentImmediate<ViewportWorldComponent>(GetActiveScene());
	DeferredTaskSystem::AddWorldComponentImmediate<TimeWorldComponent>(GetActiveScene());
	DeferredTaskSystem::AddWorldComponentImmediate<DebugWorldComponent>(GetActiveScene());
	DeferredTaskSystem::AddWorldComponentImmediate<SoundWorldComponent>(GetActiveScene(), GetActiveScene());
	DeferredTaskSystem::AddWorldComponentImmediate<DeferredTaskWorldComponent>(GetActiveScene());
	Physics2DConfig physicsConfig;
	DeferredTaskSystem::AddWorldComponentImmediate<Physics2DWorldComponent>(GetActiveScene(), physicsConfig);
	Physics3DConfig physics3DConfig;
	DeferredTaskSystem::AddWorldComponentImmediate<Physics3DWorldComponent>(GetActiveScene(), physics3DConfig);
	DeferredTaskSystem::AddWorldComponentImmediate<DebugDrawStateWorldComponent>(GetActiveScene());
}

bool Engine::IsQuitRequested() const { return QuitRequested; }
