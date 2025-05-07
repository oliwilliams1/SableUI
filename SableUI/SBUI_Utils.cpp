#include "SBUI_Utils.h"
#include <algorithm>

bool SableUI::RectBoundingBox(rect r, ivec2 p)
{
	return r.x <= p.x && r.x + r.w >= p.x && r.y <= p.y && r.y + r.h >= p.y;
}

SableUI::colour SableUI::StringTupleToColour(const char* str)
{
    int r = 255, g = 255, b = 255;
    if (str == nullptr || strlen(str) == 0) {
        return colour(r, g, b);
    }

    if (sscanf(str, "(%d, %d, %d)", &r, &g, &b) != 3)
    {
        return colour(255, 255, 255);
    }

    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);

    return colour(r, g, b);
}