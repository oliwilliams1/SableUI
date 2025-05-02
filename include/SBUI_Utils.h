#pragma once
#include <stdio.h>

struct SbUIvec3
{
    float x;
    float y;
    float z;

    SbUIvec3(float x, float y, float z) : x(x), y(y), z(z) {}
    SbUIvec3(float x)                   : x(x), y(x), z(x) {}

    SbUIvec3 operator+(const SbUIvec3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    SbUIvec3 operator-(const SbUIvec3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    SbUIvec3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    SbUIvec3 operator/(float scalar) const {
        if (scalar != 0) {
            return { x / scalar, y / scalar, z / scalar };
        }
        else {
            printf("Error: Division by zero error\n");
            return { 0, 0, 0 };
        }
    }
};

struct SbUIvec2
{
    float x;
    float y;

    SbUIvec2(float x, float y) : x(x), y(y) {}
    SbUIvec2(float x)          : x(x), y(x) {}

    SbUIvec2 operator+(const SbUIvec2& other) const {
        return { x + other.x, y + other.y };
    }

    SbUIvec2 operator-(const SbUIvec2& other) const {
        return { x - other.x, y - other.y };
    }

    SbUIvec2 operator*(float scalar) const {
        return { x * scalar, y * scalar };
    }

    SbUIvec2 operator/(float scalar) const {
        if (scalar != 0) {
            return { x / scalar, y / scalar };
        }
        else {
            printf("Error: Division by zero error\n");
			return { 0, 0 };
		}
    }
};

struct SbUIcolour
{
    uint32_t value = 0xFFFFFFFF; // ARGB

    SbUIcolour(int r, int g, int b, int a = 255) {
        value = (a << 24) | (r << 16) | (g << 8) | b;
    }
};