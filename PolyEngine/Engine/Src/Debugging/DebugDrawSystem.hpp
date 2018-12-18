#pragma once

#include <Defines.hpp>
#include <Math/Vector.hpp>
#include <Math/AABox.hpp>
#include <Math/Frustum.hpp>

namespace Poly
{
	class Scene;

	namespace DebugDrawSystem
	{
		void ENGINE_DLLEXPORT DebugRenderingUpdatePhase(Scene*);

		/// Emitters of elementary shapes
		void ENGINE_DLLEXPORT DrawLine(Scene* world, const Vector& begin, const Vector& end, const Color& color = Color::WHITE);
		void ENGINE_DLLEXPORT DrawBox(Scene* world, const Vector& mins, const Vector& maxs, const Color& color = Color::WHITE);
		void ENGINE_DLLEXPORT DrawBox(Scene* world, const Vector& mins, const Vector& maxs, const Matrix& worldFromSpace, const Color& color = Color::WHITE);
		void ENGINE_DLLEXPORT DrawBox(Scene* world, const AABox& box, const Color& color = Color::WHITE);
		
		void ENGINE_DLLEXPORT DrawFrustum(Scene* scene, const Frustum& frustum, const Matrix& viewFromWorld = Matrix(), Color color = Color::WHITE);
		void ENGINE_DLLEXPORT DrawFrustumPoints(Scene* scene, const Dynarray<Vector>& cornersInWorld, const Color color = Color::WHITE);

		/// Think of orientation as of normal vector
		void ENGINE_DLLEXPORT DrawCircle(Scene* world, const Vector& position, float radius, Vector orientation, const Color& color = Color::WHITE);
		void ENGINE_DLLEXPORT DrawSphere(Scene* world, const Vector& position, float radius, const Color& color = Color::WHITE);
		void ENGINE_DLLEXPORT DrawArrow(Scene* world, Vector position, Vector directionVector, const Color& color = Color::WHITE);

		void ENGINE_DLLEXPORT DrawText2D(Scene* world, const Vector2i& screenPosition, String text, size_t fontSize = 16, const Color& color = Color::WHITE);
	}
}