#pragma once
#include <stdio.h>

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

    struct colour
    {
        uint32_t value = 0xFFFFFFFF; // ARGB

        colour(int r, int g, int b, int a = 255) {
            value = (a << 24) | (r << 16) | (g << 8) | b;
        }
    };
}