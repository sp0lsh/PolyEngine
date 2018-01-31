#pragma once

#include "Defines.hpp"
#include "BasicMath.hpp"
#include "Vector.hpp"

// TODO: unit tests

namespace Poly {

	inline void RandomSetSeed(int value)
	{
		srand(value);
	}

	inline float Random()
	{
		float rnd = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		return rnd - floor(rnd);
	}

	inline float RandomRange(float min, float max)
	{
		float rnd = Random();
		return Lerp(min, max, rnd);
	}

	inline Vector RandomVector()
	{
		return Vector(Random(), Random(), Random());
	}

	inline Vector RandomVectorRange(float min, float max)
	{
		return Vector(RandomRange(min, max), RandomRange(min, max), RandomRange(min, max));
	}
}
