#version 330 core

in vec2 uv;
out vec4 FragColor;

layout(std140) uniform RectBlock
{
    vec4  uRect;         // x, y, w, h (NDC)
    vec4  uColour;
    vec4  uBorderColour;
    vec4  uRealRect;     // x, y, w, h (pixels)
    vec4  uRadius;       // tl, tr, bl, br
    ivec4 uBorderSize;   // t, b, l, r
    int   uUseTexture;
};

uniform sampler2D uTexture;

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
}