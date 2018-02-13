#pragma once

#include "ComponentBase.hpp"
#include "CameraSystem.hpp"

namespace Poly {

	enum class eRenderingModeType
	{
		LIT,
		UNLIT,
		WIREFRAME,
		DEBUG_NORMALS,
		DEBUG_NORMALS_WIREFRAME,
		IMMEDIATE_DEBUG,
		_COUNT
	};

	class ENGINE_DLLEXPORT CameraComponent : public ComponentBase
	{
		friend void CameraSystem::CameraUpdatePhase(World*);
	public:
		CameraComponent(Angle fov, float zNear, float zFar);
		CameraComponent(float top, float bottom, float left, float right, float zNear, float zFar);

		const Matrix& GetScreenFromView() const { return ScreenFromView; }
		const Matrix& GetViewFromWorld() const { return ViewFromWorld; }
		const Matrix& GetScreenFromWorld() const { return ScreenFromWorld; }

		const Angle& GetFOV() const { return Fov; }
		const Angle& GetTargetFOV() const { return TargetFov; }
		void SetTargetFOV(const Angle& Value) { TargetFov = Value; }
		void SetFOV(const Angle& Value) { Fov = Value; }
		float GetAspect() const { return Aspect; }
		eRenderingModeType GetRenderingMode() const { return RenderingMode; }
		void SetRenderingMode(eRenderingModeType value) { RenderingMode = value; }

	private:
		Matrix ScreenFromView;
		Matrix ViewFromWorld;
		Matrix ScreenFromWorld;

		bool IsPerspective = false;
		// Prerpective
		Angle Fov = 0_deg;
		Angle TargetFov = 0_deg;

		// Orthographic
		float Top = 0.f;
		float Bottom = 0.f;
		float Left = 0.f;
		float Right = 0.f;

		//Common
		float Near = 0.f;
		float Far = 0.f;
		float Aspect = 1.f;

		// RenderingMode
		eRenderingModeType RenderingMode;
	};

	REGISTER_COMPONENT(ComponentsIDGroup, CameraComponent)
}