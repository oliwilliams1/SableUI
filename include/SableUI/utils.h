#pragma once
#include "SableUI/console.h"

#include <stdio.h>
#include <cstdint>
#include <cmath>

namespace SableUI
{
    inline int f2i(float f) { return static_cast<int>(std::round(f)); }

    struct ivec2
    {
        int x;
        int y;

        ivec2(int x, int y) : x(x), y(y) {}
        ivec2(int x) : x(x), y(x) {}

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

    struct vec3
    {
        float x;
        float y;
        float z;

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

    struct vec2
    {
        float x;
        float y;

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

    struct Colour
    {
                        /* R G B A */
        uint32_t value = 0xFFFFFFFF;
        char r, g, b, a;

        Colour(int r, int g, int b, int a = 255) {
            value = (a << 24) | (b << 16) | (g << 8) | r; //ogl spec
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
        }
    };

    enum RectType
    {
        FILL  = 0x0,
        FIXED = 0x1
    };

    struct rect
    {
        rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
        rect(float x, float y, float w, float h, RectType wType, RectType hType)
            : x(x), y(y), w(w), h(h), wType(wType), hType(hType) {}

        float x = 0;
        float y = 0;
        float w = 0;
        float h = 0;

        RectType wType = FILL;
        RectType hType = FILL;

        bool operator !=(const rect& other) const {
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

    bool RectBoundingBox(rect r, ivec2 p);

    Colour StringTupleToColour(const char* str);
}