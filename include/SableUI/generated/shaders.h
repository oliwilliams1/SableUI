// Auto-generated shader header
#pragma once

constexpr const char rect_frag[] = R"(#version 330 core

out vec4 FragColor;

uniform vec4 uColour = vec4(vec3(0.0), 1.0);
uniform bool uUseTexture = false;
uniform vec4 uRealRect = vec4(0.0); // x, y, w, h in px
uniform float uRadius = 0.0;
uniform sampler2D uTexture;

in vec2 uv;

void main()
{
    float alpha = uColour.a;
    if (uRadius > 1.0)
    {
        vec2 fragPosPx = vec2(
            uRealRect.x + uv.x * uRealRect.z,
            uRealRect.y + uv.y * uRealRect.w
        );

        vec2 rectMin = uRealRect.xy;
        vec2 rectMax = uRealRect.xy + uRealRect.zw;

        vec2 inner = clamp(fragPosPx, rectMin + uRadius, rectMax - uRadius);
        vec2 delta = fragPosPx - inner;
        float dist = length(delta);

        alpha *= 1.0 - smoothstep(uRadius - 1.0, uRadius + 1.0, dist);
        if (alpha <= 0.0)
            discard;
    }

    vec4 baseColor = uUseTexture ? texture(uTexture, uv) : uColour;
    FragColor = vec4(baseColor.rgb, baseColor.a * alpha);
})";

constexpr const char rect_vert[] = R"(#version 330 core

layout(location = 0) in vec2 aUV;

uniform vec4 uRect; // x, y, w, h

out vec2 uv;

void main()
{
    gl_Position = vec4(uRect.xy + aUV * uRect.zw, 0.0, 1.0);
    uv = aUV;
}
)";

constexpr const char text_frag[] = R"(#version 330 core

out vec4 FragColour;

in vec3 UV;
in vec4 colour;

uniform sampler2DArray uAtlas;

void main()
{
    vec3 coverage = texture(uAtlas, UV).rgb;

    float alpha = max(max(coverage.r, coverage.g), coverage.b);

    vec3 ink = colour.rgb * coverage;
    ink = pow(ink, vec3(1.0/2.2));

    FragColour = vec4(ink, alpha * colour.a);
})";

constexpr const char text_vert[] = R"(#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aUV;
layout (location = 2) in uint aColour;

out vec3 UV;
out vec4 colour;

uniform vec2 uTargetSize;
uniform vec2 uPos;

void main()
{
	vec2 pos = (aPos + ivec2(uPos)) / uTargetSize;
	pos = pos * 2.0 - 1.0;
	pos.y *= -1.0;
	gl_Position = vec4(pos, 0.0, 1.0);
	UV = aUV;

	colour = vec4(
        float((aColour      ) & 0xFFu) / 255.0,
        float((aColour >> 8)  & 0xFFu) / 255.0,
        float((aColour >> 16) & 0xFFu) / 255.0,
        float((aColour >> 24) & 0xFFu) / 255.0
    );
})";

