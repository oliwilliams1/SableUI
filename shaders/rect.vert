#version 420 core

layout(location = 0) in vec2 aUV;
out vec2 uv;

layout(std140, binding = 1) uniform RectBlock
{
	vec4  uRect;         // x, y, w, h (NDC)
	vec4  uColour;
	vec4  uBorderColour;
	vec4  uRealRect;     // x, y, w, h (pixels)
	vec4  uRadius;       // tl, tr, bl, br
	ivec4 uBorderSize;   // t, b, l, r
	int   uUseTexture;
};

void main()
{
	gl_Position = vec4(uRect.xy + aUV * uRect.zw, 0.0, 1.0);
	uv = aUV;
}
