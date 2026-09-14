#include <SableUI/utils/utils.h>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <vector>

bool SableUI::RectBoundingBox(const Rect& r, const ivec2& p, const std::vector<Obscurer>& obscurers, int z)
{
	if (!RectBoundingBox(r, p))
		return false;

	for (const Obscurer& o : obscurers)
		if (o.z > z && RectBoundingBox(o.r, p))
			return false;

	return true;
}

SableUI::Colour SableUI::StringTupleToColour(const char* str)
{
	int r = 255, g = 255, b = 255;
	if (str == nullptr || strlen(str) == 0)
		return Colour(r, g, b);

	char discard;

	// parse string into r, g, b input = (r, g, b)
	std::istringstream iss(str);
	if (!(iss >> discard >> r >> discard >> g >> discard >> b) || discard != ',')
		return Colour(255, 255, 255);

	// prevent overflows
	r = std::clamp(r, 0, 255);
	g = std::clamp(g, 0, 255);
	b = std::clamp(b, 0, 255);

	return Colour(r, g, b);
}