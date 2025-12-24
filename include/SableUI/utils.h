#pragma once
#include <SableUI/string.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cmath>

typedef SableUI::String SableString;

namespace SableUI
{
	inline int f2i(float f) { return static_cast<int>(std::round(f)); }

	struct uvec2
	{
		unsigned int x = 0;
		unsigned int y = 0;

		uvec2() = default;
		uvec2(unsigned x, unsigned y) : x(x), y(y) {}
		uvec2(unsigned x) : x(x), y(x) {}

		bool operator==(const uvec2& other) const {
			return x == other.x && y == other.y;
		}

		uvec2 operator+(const uvec2& other) const {
			return { x + other.x, y + other.y };
		}

		uvec2 operator-(const uvec2& other) const {
			return { x - other.x, y - other.y };
		}

		uvec2 operator*(unsigned scalar) const {
			return { x * scalar, y * scalar };
		}

		uvec2 operator/(unsigned scalar) const {
			if (scalar != 0) {
				return { x / scalar, y / scalar };
			}
			else {
				printf("Error: Division by zero error\n");
				return { 0, 0 };
			}
		}
	};

	struct ivec2
	{
		union {
			struct { int x, y; };
			struct { int w, h; };
		};

		ivec2() : x(0), y(0) {};
		ivec2(int x, int y) : x(x), y(y) {}
		ivec2(int x) : x(x), y(x) {}
		ivec2(uvec2 v) : x(v.x), y(v.y) {}

		bool operator==(const ivec2& other) const {
			return x == other.x && y == other.y;
		}

		ivec2 operator+(const ivec2& other) const {
			return { x + other.x, y + other.y };
		}

		ivec2 operator-(const ivec2& other) const {
			return { x - other.x, y - other.y };
		}

		ivec2 operator*(int scalar) const {
			return { x * scalar, y * scalar };
		}

		ivec2 operator/(int scalar) const {
			if (scalar != 0) {
				return { x / scalar, y / scalar };
			}
			else {
				printf("Error: Division by zero error\n");
				return { 0, 0 };
			}
		}
	};

	struct u16vec2
	{
		uint16_t x = 0;
		uint16_t y = 0;

		u16vec2() = default;
		u16vec2(uint16_t x, uint16_t y) : x(x), y(y) {}
		u16vec2(int x, int y) : x(static_cast<uint16_t>(x)), y(static_cast<uint16_t>(y)) {}
		u16vec2(uint16_t x) : x(x), y(x) {}

		bool operator==(const u16vec2& other) const {
			return x == other.x && y == other.y;
		}

		u16vec2 operator+(const u16vec2& other) const {
			return { x + other.x, y + other.y };
		}

		u16vec2 operator-(const u16vec2& other) const {
			return { x - other.x, y - other.y };
		}

		u16vec2 operator*(uint16_t scalar) const {
			return { x * scalar, y * scalar };
		}

		u16vec2 operator/(uint16_t scalar) const {
			if (scalar != 0) {
				return { x / scalar, y / scalar };
			}
			else {
				printf("Error: Division by zero error\n");
				return { 0, 0 };
			}
		}
	};

	struct vec2
	{
		float x = 0;
		float y = 0;

		bool operator==(const vec2& other) const {
			return x == other.x && y == other.y;
		}

		vec2() = default;
		vec2(float x, float y) : x(x), y(y) {}
		vec2(float x)          : x(x), y(x) {}
		vec2(int x, int y)     : x(static_cast<float>(x)),    y(static_cast<float>(y)) {}
		vec2(int x)            : x(static_cast<float>(x)),    y(static_cast<float>(x)) {}
		vec2(ivec2 v)          : x(static_cast<float>(v.x)), y(static_cast<float>(v.y)) {}

		vec2 operator+(const vec2& other) const {
			return { x + other.x, y + other.y };
		}

		vec2 operator-(const vec2& other) const {
			return { x - other.x, y - other.y };
		}

		vec2 operator*(float scalar) const {
			return { x * scalar, y * scalar };
		}

		vec2 operator/(float scalar) const {
			if (scalar != 0) {
				return { x / scalar, y / scalar };
			}
			else {
				printf("Error: Division by zero error\n");
				return { 0, 0 };
			}
		}

		void normalise()
		{
			float f = std::sqrt(x * x + y * y);
			x /= f;
			y /= f;
		}
	};

	struct vec3
	{
		float x = 0;
		float y = 0;
		float z = 0;

		bool operator==(const vec3& other) const {
			return x == other.x && y == other.y && z == other.z;
		}

		vec3() = default;
		vec3(float x, float y, float z) : x(x), y(y), z(z) {}
		vec3(float x) : x(x), y(x), z(x) {}

		vec3 operator+(const vec3& other) const {
			return { x + other.x, y + other.y, z + other.z };
		}

		vec3 operator-(const vec3& other) const {
			return { x - other.x, y - other.y, z - other.z };
		}

		vec3 operator*(float scalar) const {
			return { x * scalar, y * scalar, z * scalar };
		}

		vec3 operator/(float scalar) const {
			if (scalar != 0) {
				return { x / scalar, y / scalar, z / scalar };
			}
			else {
				printf("Error: Division by zero error\n");
				return { 0, 0, 0 };
			}
		}
	};

	struct vec4
	{
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;

		bool operator==(const vec4& other) const {
			return x == other.x && y == other.y && z == other.z && w == other.w;
		}

		vec4() = default;
		vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		vec4(float value) : x(value), y(value), z(value), w(value) {}

		vec4 operator+(const vec4& other) const {
			return { x + other.x, y + other.y, z + other.z, w + other.w };
		}

		vec4 operator-(const vec4& other) const {
			return { x - other.x, y - other.y, z - other.z, w - other.w };
		}

		vec4 operator*(float scalar) const {
			return { x * scalar, y * scalar, z * scalar, w * scalar };
		}

		vec4 operator/(float scalar) const {
			if (scalar != 0) {
				return { x / scalar, y / scalar, z / scalar, w / scalar };
			}
			else {
				printf("Error: Division by zero\n");
				return { 0, 0, 0, 0 };
			}
		}
	};

	struct Colour
	{
		uint8_t r = 0, g = 0, b = 0, a = 255;

		constexpr bool operator==(const Colour& other) const
		{
			return r == other.r && g == other.g &&
				b == other.b && a == other.a;
		}

		Colour& operator=(const Colour& other)
		{
			if (this != &other)
			{
				r = other.r;
				g = other.g;
				b = other.b;
				a = other.a;
			}
			return *this;
		}
	};

	typedef Colour Color;

	enum RectType
	{
		Undef       = 0x0,
		Fill        = 0x1,
		Fixed       = 0x2,
		FitContent	= 0x3
	};

	struct Rect
	{
		Rect() : x(0), y(0), w(0), h(0) {}
		Rect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

		union
		{
			struct
			{
				int x;
				int y;
				int w;
				int h;
			};
			struct
			{
				int xPos;
				int yPos;
				int width;
				int height;
			};
		};

		bool operator==(const Rect& other) const {
			return x == other.x && y == other.y && w == other.w && h == other.h;
		}

		bool operator!=(const Rect& other) const {
			return x != other.x || y != other.y || w != other.w || h != other.h;
		}

		SableString ToString() const {
			return SableString::Format("{ x: %d, y: %d, width: %d, height: %d }", x, y, w, h);
		}

		bool intersect(const Rect& other) const {
			if (x + w <= other.x) return false;
			if (other.x + other.w <= x) return false;

			if (y + h <= other.y) return false;
			if (other.y + other.h <= y) return false;

			return true;
		}

		Rect getIntersection(const Rect& other) {
			int newX = (std::max)(x, other.x);
			int newY = (std::max)(y, other.y);
			int newRight = (std::min)(x + w, other.x + other.w);
			int newBottom = (std::min)(y + h, other.y + other.h);
			return { newX, newY, (std::max)(0, newRight - newX), (std::max)(0, newBottom - newY) };
		}
	};

	enum EdgeType
	{
		NS_EDGE,
		EW_EDGE,
		NONE
	};

	bool RectBoundingBox(Rect r, ivec2 p);

	Colour StringTupleToColour(const char* str);
}