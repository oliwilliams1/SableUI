// Auto-generated shader header
#pragma once

constexpr const char rect_frag[] = R"(#version 420 core

in vec2 uv;
out vec4 FragColor;

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

layout(binding = 0) uniform sampler2D uTexture;

float GetRadius(vec2 p, vec2 center, vec4 r)
{
    vec2 quadrant = step(center, p);
    return mix(
        mix(r.x, r.z, quadrant.y), 
        mix(r.y, r.w, quadrant.y), 
        quadrant.x
    );
}

float RoundedRectDist(vec2 p, vec2 minP, vec2 maxP, vec4 r)
{
    vec2 center = (minP + maxP) * 0.5;
    vec2 h = (maxP - minP) * 0.5;
    vec2 q = abs(p - center) - h;

    float radius = GetRadius(p, center, r);
    q += vec2(radius);

    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
}

void main()
{
    float w = uRealRect.z;
    float h = uRealRect.w;

    vec4 r = max(uRadius, 0.0);
    float maxR = 0.5 * min(w, h);
    r = min(r, vec4(maxR));

    float topSum = r.x + r.y;
    if (topSum > w) r.xy *= w / topSum;

    float bottomSum = r.z + r.w;
    if (bottomSum > w) r.zw *= w / bottomSum;

    float leftSum = r.x + r.z;
    if (leftSum > h) r.xz *= h / leftSum;

    float rightSum = r.y + r.w;
    if (rightSum > h) r.yw *= h / rightSum;

    vec2 fragPosPx = uRealRect.xy + uv * uRealRect.zw;
    vec2 rectMin = uRealRect.xy;
    vec2 rectMax = rectMin + uRealRect.zw;

    vec4 border = vec4(uBorderSize); // t, b, l, r

    vec2 innerMin = rectMin + vec2(border.z, border.x); // left, top
    vec2 innerMax = rectMax - vec2(border.w, border.y); // right, bottom

    vec4 innerR = r;
    innerR.x = max(0.0, r.x - min(border.x, border.z));
    innerR.y = max(0.0, r.y - min(border.x, border.w));
    innerR.z = max(0.0, r.z - min(border.y, border.z));
    innerR.w = max(0.0, r.w - min(border.y, border.w));

    float distOuter = RoundedRectDist(fragPosPx, rectMin, rectMax, r);
    float alphaOuter = 1.0 - smoothstep(-0.5, 0.5, distOuter);
    if (alphaOuter <= 0.0) discard;
    bool hasBorder = dot(vec4(uBorderSize), vec4(1.0)) > 0.0;
    
    float borderFactor = 0.0;
    if (hasBorder) {
        float distInner = RoundedRectDist(fragPosPx, innerMin, innerMax, innerR);
        borderFactor = smoothstep(-0.5, 0.5, distInner); 
    }

    vec4 fillCol = bool(uUseTexture) ? texture(uTexture, uv) : uColour;
    vec4 finalColor = mix(fillCol, uBorderColour, borderFactor);
    finalColor.a *= alphaOuter;
    
    FragColor = finalColor;
})";

constexpr const char rect_vert[] = R"(#version 420 core

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
)";

constexpr const char text_frag[] = R"(#version 420 core

out vec4 FragColour;

in vec3 UV;
in vec4 colour;

layout(binding = 0) uniform sampler2DArray uAtlas;

void main()
{
	float a = texture(uAtlas, UV).r;

	a = a * a * (3.0 - 2.0 * a);

	FragColour = vec4(colour.rgb, a * colour.a);
})";

constexpr const char text_vert[] = R"(#version 420 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aUV;
layout (location = 2) in uint aColour;

out vec3 UV;
out vec4 colour;

layout(std140, binding = 2) uniform TextBlock
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
})";

