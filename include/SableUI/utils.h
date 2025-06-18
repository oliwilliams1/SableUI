#pragma once
#include "SableUI/console.h"

#include <stdio.h>
#include <cstdint>
#include <cmath>

namespace SableUI
{
    inline int f2i(float f) { return static_cast<int>(std::round(f)); }

    struct uvec2
    {
        unsigned int x;
        unsigned int y;

        uvec2() = default;
        uvec2(unsigned x, unsigned y) : x(x), y(y) {}
        uvec2(unsigned x) : x(x), y(x) {}

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
        int x;
        int y;

        ivec2() = default;
        ivec2(int x, int y) : x(x), y(y) {}
        ivec2(int x) : x(x), y(x) {}
        ivec2(uvec2 v) : x(v.x), y(v.y) {}

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

    struct vec2
    {
        float x;
        float y;

        vec2() = default;
        vec2(float x, float y) : x(x), y(y) {}
        vec2(float x)          : x(x), y(x) {}
        vec2(int x, int y)     : x(static_cast<float>(x)),   y(static_cast<float>(y)) {}
		vec2(int x)            : x(static_cast<float>(x)),   y(static_cast<float>(x)) {}
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
    };

    struct vec3
    {
        float x;
        float y;
        float z;

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
        float x;
        float y;
        float z;
        float w;

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
        Colour(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
        
        uint8_t r, g, b, a;
    };

    enum RectType
    {
        FILL  = 0x0,
        FIXED = 0x1
    };

    struct Rect
    {
        Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
        Rect(float x, float y, float w, float h, RectType wType, RectType hType)
            : x(x), y(y), w(w), h(h), wType(wType), hType(hType) {}

        float x = 0;
        float y = 0;
        float w = 0;
        float h = 0;

        RectType wType = FILL;
        RectType hType = FILL;

        bool operator !=(const Rect& other) const {
			return x != other.x || y != other.y || w != other.w || h != other.h;
		}

        void print() const
        {
            printf("x: %f, y: %f, w: %f, h: %f\n", x, y, w, h);
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