#version 330 core

out vec4 FragColor;

uniform vec4 uColour = vec4(vec3(0.0), 1.0);
uniform bool uUseTexture = false;
uniform vec4 uRealRect = vec4(0.0); // x, y, w, h in px
uniform vec4 uRadius = vec4(0.0);   // tl, tr, bl, br in px
uniform sampler2D uTexture;

in vec2 uv;

void main()
{
    // remove gamma correction for alpha
    float alpha = pow(uColour.a, 1.0 / 2.2);

    float w = uRealRect.z;
    float h = uRealRect.w;

    // clamp radii
    vec4 r = max(uRadius, 0.0);
    float maxR = 0.5 * min(w, h);
    r = min(r, vec4(maxR));

    // prevent overlap horizontally
    float topSum = r.x + r.y;
    if (topSum > w) r.xy *= w / topSum;

    float bottomSum = r.z + r.w;
    if (bottomSum > w) r.zw *= w / bottomSum;

    // prevent overlap vertically
    float leftSum = r.x + r.z;
    if (leftSum > h) r.xz *= h / leftSum;

    float rightSum = r.y + r.w;
    if (rightSum > h) r.yw *= h / rightSum;

    vec2 fragPosPx = uRealRect.xy + uv * uRealRect.zw;
    vec2 rectMin = uRealRect.xy;
    vec2 rectMax = rectMin + uRealRect.zw;

    float dist = 0.0;

    // top-left
    if (fragPosPx.x < rectMin.x + r.x &&
        fragPosPx.y < rectMin.y + r.x)
    {
        dist = length(fragPosPx - (rectMin + vec2(r.x))) - r.x;
    }
    // top-right
    else if (fragPosPx.x > rectMax.x - r.y &&
             fragPosPx.y < rectMin.y + r.y)
    {
        dist = length(fragPosPx - (vec2(rectMax.x - r.y, rectMin.y + r.y))) - r.y;
    }
    // bottom-left
    else if (fragPosPx.x < rectMin.x + r.z &&
             fragPosPx.y > rectMax.y - r.z)
    {
        dist = length(fragPosPx - (vec2(rectMin.x + r.z, rectMax.y - r.z))) - r.z;
    }
    // bottom-right
    else if (fragPosPx.x > rectMax.x - r.w &&
             fragPosPx.y > rectMax.y - r.w)
    {
        dist = length(fragPosPx - (rectMax - vec2(r.w))) - r.w;
    }
    else
    {
        // inside straight edges
        dist = -1.0;
    }

    // AA
    alpha *= 1.0 - smoothstep(-1.0, 1.0, dist);
    if (alpha <= 0.0)
        discard;

    vec4 baseColor = uUseTexture ? texture(uTexture, uv) : uColour;
    FragColor = vec4(baseColor.rgb, baseColor.a * alpha);
}
