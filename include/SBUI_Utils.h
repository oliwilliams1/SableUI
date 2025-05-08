#pragma once
#include <stdio.h>
#include <cstdint>

namespace SableUI
{
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
        vec2(float x) : x(x), y(x) {}

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

    struct colour
    {
        uint32_t value = 0xFFFFFFFF; // ARGB

        colour(int r, int g, int b, int a = 255) {
            value = (a << 24) | (r << 16) | (g << 8) | b;
        }
    };

    struct rect
    {
        rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
        float x = 0;
        float y = 0;
        float w = 0;
        float h = 0;
    };

    enum EdgeType {
        NS_EDGE,
        EW_EDGE,
        NONE
    };

    bool RectBoundingBox(rect r, ivec2 p);

    colour StringTupleToColour(const char* str);
}