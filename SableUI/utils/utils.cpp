#include <SableUI/utils/utils.h>
#include <algorithm>
#include <sstream>
#include <cstring>

bool SableUI::RectBoundingBox(Rect r, ivec2 p)
{
	return r.x <= p.x && r.x + r.w >= p.x && r.y <= p.y && r.y + r.h >= p.y;
}

SableUI::Colour SableUI::StringTupleToColour(const char* str)
{
    int r = 255, g = 255, b = 255;
    if (str == nullptr || strlen(str) == 0) {
        return Colour(r, g, b);
    }

    char discard;

    /* parse string into r, g, b input = (r, g, b) */
    std::istringstream iss(str);
    if (!(iss >> discard >> r >> discard >> g >> discard >> b) || discard != ',') {
        return Colour(255, 255, 255);
    }

    /* prevent overflows */
    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);

    return Colour(r, g, b);
}