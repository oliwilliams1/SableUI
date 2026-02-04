#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aUV;
layout (location = 2) in uint aColour;

out vec3 UV;
out vec4 colour;

layout(std140) uniform TextBlock
{
	vec2 uTargetSize;
	vec2 uPos;
};

void main()
{
	vec2 pos = (aPos + ivec2(uPos)) / uTargetSize;
	pos = pos * 2.0 - 1.0;
	pos.y *= -1.0;
	gl_Position = vec4(pos, 0.0, 1.0);
	UV = aUV;

	colour = vec4(
		float((aColour      ) & 0xFFu) / 255.0,
		float((aColour >>  8) & 0xFFu) / 255.0,
		float((aColour >> 16) & 0xFFu) / 255.0,
		float((aColour >> 24) & 0xFFu) / 255.0
	);
}