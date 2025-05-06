#include "SBUI_Utils.h"
#include <algorithm>

bool SableUI::RectBoundingBox(rect r, ivec2 p)
{
	return r.x <= p.x && r.x + r.w >= p.x && r.y <= p.y && r.y + r.h >= p.y;
}