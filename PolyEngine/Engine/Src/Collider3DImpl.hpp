#pragma once

#include <BaseObject.hpp>

class btCollisionObject;

namespace Poly
{
	struct Trigger3DImpl : BaseObject<>
	{
		btCollisionObject* BulletTrigger = nullptr;
	};
} // namespace Poly