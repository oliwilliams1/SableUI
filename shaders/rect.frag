#version 330 core

out vec4 FragColor;

uniform vec4 uColour = vec4(vec3(0.0), 1.0);
uniform bool uUseTexture = false;
uniform vec4 uRealRect = vec4(0.0); // x, y, w, h in px
uniform float uRadius = 0.0;
uniform sampler2D uTexture;

in vec2 uv;

void main()
{
    // remove gamma correction for alpha
	float alpha = pow(uColour.a, 1.0 / 2.2);

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
}