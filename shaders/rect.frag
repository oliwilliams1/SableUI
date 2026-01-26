#version 330 core

out vec4 FragColor;

uniform vec4 uColour = vec4(vec3(0.0), 1.0);
uniform bool uUseTexture = false;
uniform vec4 uRealRect = vec4(0.0);		// x, y, w, h in px
uniform vec4 uRadius = vec4(0.0);		// tl, tr, bl, br in px
uniform ivec4 uBorderSize = ivec4(0);	// t, b, l, r in px
uniform vec3 uBorderColour = vec3(0.0); // rgb
uniform sampler2D uTexture;

in vec2 uv;

float RoundedRectDist(vec2 p, vec2 minP, vec2 maxP, vec4 r)
{
	vec2 center = (minP + maxP) * 0.5;
	vec2 h	= (maxP - minP) * 0.5;

	vec2 q = abs(p - center) - h;

	float radius;
	if (p.x < center.x && p.y < center.y)		radius = r.x; // tl
	else if (p.x > center.x && p.y < center.y)	radius = r.y; // tr
	else if (p.x < center.x && p.y > center.y)	radius = r.z; // bl
	else										radius = r.w; // br

	q += vec2(radius);

	return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - radius;
}

void main()
{
	// remove gamma correction for alpha
	float alpha = pow(uColour.a, 1.0 / 2.2);

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

	bool hasBorder =
		(uBorderSize.x > 0) ||
		(uBorderSize.y > 0) ||
		(uBorderSize.z > 0) ||
		(uBorderSize.w > 0);

	vec2 innerMin = rectMin + vec2(border.z, border.x);
	vec2 innerMax = rectMax - vec2(border.w, border.y);

	vec4 innerR = r;
	innerR.x = max(0.0, r.x - min(border.x, border.z));
	innerR.y = max(0.0, r.y - min(border.x, border.w));
	innerR.z = max(0.0, r.z - min(border.y, border.z));
	innerR.w = max(0.0, r.w - min(border.y, border.w));

	float distOuter = RoundedRectDist(fragPosPx, rectMin, rectMax, r);
	float distInner = RoundedRectDist(fragPosPx, innerMin, innerMax, innerR);

	float outerMask = 1.0 - smoothstep(-1.0, 1.0, distOuter);

	float innerMask = hasBorder
		? (1.0 - smoothstep(-1.0, 1.0, distInner))
		: 0.0;

	float borderMask  = hasBorder ? (outerMask * (1.0 - innerMask)) : 0.0;
	float contentMask = outerMask;

	if (contentMask * alpha <= 0.0)
		discard;

	vec4 baseColor = uUseTexture ? texture(uTexture, uv) : uColour;

	vec3 finalRGB = mix(baseColor.rgb, uBorderColour.rgb, borderMask);
	float finalAlpha = contentMask * baseColor.a * alpha;

	FragColor = vec4(finalRGB, finalAlpha);
}
