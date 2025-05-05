#include "SBUI_Utils.h"
#include <algorithm>

bool SableUI::RectBoundingBox(rect r, ivec2 p)
{
	return r.x <= p.x && r.x + r.w >= p.x && r.y <= p.y && r.y + r.h >= p.y;
}

float SableUI::DistToEdge(rect r, ivec2 p, EdgeType& edgeType)
{
    float distLeft = p.x - r.x;
    float distRight = (r.x + r.w) - p.x;
    float distTop = p.y - r.y;
    float distBottom = (r.y + r.h) - p.y;

    float minDist = std::min({ distLeft, distRight, distTop, distBottom });

    if (minDist == distLeft || minDist == distRight) {
        edgeType = EW_EDGE;
    }
    else if (minDist == distTop || minDist == distBottom) {
        edgeType = NS_EDGE;
    }
    else {
        edgeType = NONE;
    }

    return (minDist < 0) ? 0 : minDist;
}