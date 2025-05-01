#pragma once
#include <stdio.h>

struct SBUIvec3
{
    float x;
    float y;
    float z;

    SBUIvec3(float x, float y, float z) : x(x), y(y), z(z) {}
    SBUIvec3(float x)                   : x(x), y(x), z(x) {}

    SBUIvec3 operator+(const SBUIvec3& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    SBUIvec3 operator-(const SBUIvec3& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    SBUIvec3 operator*(float scalar) const {
        return { x * scalar, y * scalar, z * scalar };
    }

    SBUIvec3 operator/(float scalar) const {
        if (scalar != 0) {
            return { x / scalar, y / scalar, z / scalar };
        }
        else {
            printf("Error: Division by zero error\n");
            return { 0, 0, 0 };
        }
    }
};

struct SBUIvec2
{
    float x;
    float y;

    SBUIvec2(float x, float y) : x(x), y(y) {}
    SBUIvec2(float x)          : x(x), y(x) {}

    SBUIvec2 operator+(const SBUIvec2& other) const {
        return { x + other.x, y + other.y };
    }

    SBUIvec2 operator-(const SBUIvec2& other) const {
        return { x - other.x, y - other.y };
    }

    SBUIvec2 operator*(float scalar) const {
        return { x * scalar, y * scalar };
    }

    SBUIvec2 operator/(float scalar) const {
        if (scalar != 0) {
            return { x / scalar, y / scalar };
        }
        else {
            printf("Error: Division by zero error\n");
			return { 0, 0 };
		}
    }
};
