#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aUV;

out vec3 UV;

uniform vec2 uTargetSize;
uniform vec2 uPos;

void main()
{
	vec2 pos = (aPos + ivec2(uPos)) / uTargetSize;
	pos = pos * 2.0 - 1.0;
	pos.y *= -1.0;
	gl_Position = vec4(pos, 0.0, 1.0);
	UV = aUV;
}