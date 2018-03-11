#pragma once

#include "Defines.hpp"
#include "BasicMath.hpp"

namespace Poly {
	
	/// <summary>Class representing RGBA color.</summary>
	class CORE_DLLEXPORT Color final : public BaseObjectLiteralType<>
	{
	public:
		static const Color WHITE;
		static const Color BLACK;
		static const Color RED;
		static const Color GREEN;
		static const Color BLUE;

		Color() : R(0), G(0), B(0), A(1)
		{
		}

		constexpr Color(float r, float g, float b, float a = 1.0f) : R(r), G(g), B(b), A(a)
		{
		}

		constexpr Color operator+(const Color& rhs) const {
			return Color(R + rhs.R, G + rhs.G, B + rhs.B, A + rhs.A);
		}

		constexpr Color operator-(const Color& rhs) const {
			return Color(R - rhs.R, G - rhs.G, B - rhs.B, A - rhs.A);
		}

		constexpr Color operator*(const Color& rhs) const {
			return Color(R * rhs.R, G * rhs.G, B * rhs.B, A * rhs.A);
		}

		Color& operator+=(const Color& rhs) {
			R += rhs.R; G += rhs.G; B += rhs.B; A += rhs.A;
			return *this;
		}

		Color& operator-=(const Color& rhs) {
			R -= rhs.R; G -= rhs.G; B -= rhs.B; A -= rhs.A;
			return *this;
		}

		CORE_DLLEXPORT friend std::ostream& operator<< (std::ostream& stream, const Color& color);

		union {
			float Data[4];
			struct
			{
				float R, G, B, A;
			};
		};
	};
}