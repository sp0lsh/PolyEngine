#include "PostprocessRenderingPass.hpp"

#include "GLRenderingDevice.hpp"
#include "GLMeshDeviceProxy.hpp"
#include "GLTextureDeviceProxy.hpp"

#include <AARect.hpp>
#include <World.hpp>
#include <TimeSystem.hpp>
#include <CameraComponent.hpp>
#include <MovementSystem.hpp>
#include <MeshRenderingComponent.hpp>
#include <PostprocessSettingsComponent.hpp>


using namespace Poly;

PostprocessRenderingPass::PostprocessRenderingPass(const PostprocessQuad* quad, const String& fragment)
	: RenderingPassBase("Shaders/postprocessCommonVert.shader", fragment), Quad(quad)
{
}

PostprocessRenderingPass::PostprocessRenderingPass(const PostprocessQuad* quad, const String& geometry, const String& fragment)
	: RenderingPassBase("Shaders/postprocessCommonVert.shader", geometry, fragment), Quad(quad)
{
}


void PostprocessRenderingPass::OnRun(World* world, const CameraComponent* camera, const AARect& rect, ePassType /*passType = ePassType::GLOBAL*/)
{
	float Time = (float)TimeSystem::GetTimerElapsedTime(world, eEngineTimer::GAMEPLAY);
	float ResolutionX = rect.GetSize().X * gRenderingDevice->GetScreenSize().Width;
	float ResolutionY = rect.GetSize().Y * gRenderingDevice->GetScreenSize().Height;
	const EntityTransform& cameraTransform = camera->GetTransform();
	Vector CameraPosition = cameraTransform.GetGlobalTranslation();
	Matrix CameraRotation = cameraTransform.GetGlobalRotation().ToRotationMatrix();

	GetProgram().BindProgram();

	GetProgram().SetUniform("uTime", Time);
	GetProgram().SetUniform("uResolution", ResolutionX, ResolutionY);

	GetProgram().SetUniform("uCameraPosition", CameraPosition);
	GetProgram().SetUniform("uCameraRotation", CameraRotation);

	const PostprocessSettingsComponent* PostprocessSettings = camera->GetSibling<PostprocessSettingsComponent>();
	if (PostprocessSettings == nullptr)
	{
		gConsole.LogInfo("void PostprocessRenderingPass::OnRun: PostprocessSettings is NULL");
	}
	else
	{
		GetProgram().SetUniform("uUseCashetes",						PostprocessSettings->UseCashetes);
		GetProgram().SetUniform("uDistortionPower",					PostprocessSettings->Distortion);
		GetProgram().SetUniform("uColorTempValue",					PostprocessSettings->ColorTempValue);
		GetProgram().SetUniform("uColorTempLuminancePreservation",	PostprocessSettings->ColorTempLuminancePreservation);
		GetProgram().SetUniform("uSaturationPower",					PostprocessSettings->Saturation);
		GetProgram().SetUniform("uGrainPower",						PostprocessSettings->Grain);
		GetProgram().SetUniform("uStripesPower",					PostprocessSettings->Stripes);
		
		GetProgram().SetUniform("uShipPos", PostprocessSettings->ShipPos);
		GetProgram().SetUniform("uShipAngleY", PostprocessSettings->ShipAngleY);

		GetProgram().SetUniform("uTimeOfDeath", PostprocessSettings->TimeOfDeath);
		GetProgram().SetUniform("uTimeOfAction", PostprocessSettings->TimeOfAction);

		GetProgram().SetUniform("uEnableBoss", PostprocessSettings->EnableBoss);
		
		const int MaxEnemies = 5;
		int EnemyCount = PostprocessSettings->EnemyShipPos.GetSize();
		EnemyCount = Clamp(EnemyCount, 0, MaxEnemies);
		GetProgram().SetUniform("uEnemyCount", EnemyCount);
		// gConsole.LogInfo("void PostprocessRenderingPass::OnRun: EnemyCount: {}", EnemyCount);

		if (EnemyCount > 0) {
			GetProgram().SetUniform("uEnemyShipPos0", PostprocessSettings->EnemyShipPos[0]);
			GetProgram().SetUniform("uEnemyShipAngleY0", PostprocessSettings->EnemyShipAngleY[0]);
			// gConsole.LogInfo("void PostprocessRenderingPass::OnRun: EnemyID: {}, NamePos: {}, Pos: {}, NameAngle: {}, Angle: {}",
			// 	0, PostprocessSettings->EnemyShipPos[0], PostprocessSettings->EnemyShipAngleY[0]);
		}

		if (EnemyCount > 1) {
			GetProgram().SetUniform("uEnemyShipPos1", PostprocessSettings->EnemyShipPos[1]);
			GetProgram().SetUniform("uEnemyShipAngleY1", PostprocessSettings->EnemyShipAngleY[1]);
			// gConsole.LogInfo("void PostprocessRenderingPass::OnRun: EnemyID: {}, NamePos: {}, Pos: {}, NameAngle: {}, Angle: {}",
			// 	0, PostprocessSettings->EnemyShipPos[0], PostprocessSettings->EnemyShipAngleY[0]);
		}
		/*
		for (int i = 1; i < EnemyCount; ++i)
		{
			String namePos = String("uEnemyShipPos[") + String::From(i) + String("]");
			String nameAngle = String("uEnemyShipAngleY[") + String::From(i) + String("]");
			// String namePos = String("uEnemyShipPos");
			// String nameAngle = String("uEnemyShipAngleY");
			GetProgram().SetUniform(namePos, PostprocessSettings->EnemyShipPos[i]);
			GetProgram().SetUniform(nameAngle, PostprocessSettings->EnemyShipAngleY[i]);
			gConsole.LogInfo("void PostprocessRenderingPass::OnRun: EnemyID: {}, NamePos: {}, Pos: {}, NameAngle: {}, Angle: {}",
				i, namePos, PostprocessSettings->EnemyShipPos[i], nameAngle, PostprocessSettings->EnemyShipAngleY[i]);
		}
		*/
		
		//gConsole.LogInfo("void PostprocessRenderingPass::OnRun: UseCashetes: {}", PostprocessSettings->UseCashetes);
	}


	glBindVertexArray(Quad->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
