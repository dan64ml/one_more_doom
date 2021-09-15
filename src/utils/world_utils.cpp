#include "world_utils.h"
#include "renderer/plane_utils.h"

namespace math {

ObjPosition LinePointPosition(const world::Line* line, int x, int y) {
  int dx = line->x2 - line->x1;
  int dy = line->y2 - line->y1;

  if (!dx) {
    if (x <= line->x1) {
      return (dy > 0) ? kLeftSide : kRightSide;
    } else {
      return (dy < 0) ? kLeftSide : kRightSide;
    }
  }

  if (!dy) {
    if (y <= line->y1) {
      return (dx < 0) ? kLeftSide : kRightSide;
    } else {
      return (dx > 0) ? kLeftSide : kRightSide;
    }
  }

  double dvp_x = x - line->x1;
  double dvp_y = y - line->y1;
  double area = dx * dvp_y - dvp_x * dy;

  return (area > 0) ? kLeftSide : kRightSide;
}

// Considers that the line is infinite
ObjPosition LineBBoxPosition(const world::Line* line, const world::BBox* bbox) {
  int dx = line->x2 - line->x1;
  int dy = line->y2 - line->y1;

  int p1p, p2p;

  if (dy == 0) {
    // horizontal line
    p1p = bbox->top > line->y1;
    p2p = bbox->bottom > line->y1;
    if (dx < 0) {
      p1p ^= 1;
      p2p ^= 1;
    }
  } else if (dx == 0) {
    // vertical line
    p1p = bbox->right < line->x1;
    p2p = bbox->left < line->x1;
    if (dy < 0) {
      p1p ^= 1;
      p2p ^= 1;
    }
  } else if (dx*dy > 0 ) {
    p1p = LinePointPosition(line, bbox->left, bbox->top);
    p2p = LinePointPosition(line, bbox->right, bbox->bottom);
  } else {
    p1p = LinePointPosition(line, bbox->right, bbox->top);
    p2p = LinePointPosition(line, bbox->left, bbox->bottom);
  }

  if (p1p == p2p) {
    return p1p ? kLeftSide : kRightSide;
  }

  return kCross;
}

bool IsLineIntersectMobj(int x1, int y1, int x2, int y2, const mobj::MapObject* mobj) {
  
}

double GetDistanceToIntersection(int x1, int y1, int x2, int y2, const mobj::MapObject* mobj) {
  world::BBox bb {x1, x2, y1, y2};
  if (bb.left > bb.right) {
    std::swap(bb.left, bb.right);
  }
  if (bb.top < bb.bottom) {
    std::swap(bb.top, bb.bottom);
  }

  // Definitely don't cross
  if (mobj->x - mobj->radius > bb.right || mobj->x + mobj->radius < bb.left) {
    return -1;
  }
  if (mobj->y + mobj->radius < bb.bottom || mobj->y - mobj->radius > bb.top) {
    return -1;
  }

  auto center_angle = rend::CalcAngle(x1, y1, mobj->x, mobj->y);
  auto cp_angle = rend::CalcAngle(x1, y1, x2, y2);
  double center_dist = rend::SegmentLength(x1, y1, mobj->x, mobj->y);
  double cp_dist = center_dist * rend::BamTan(center_angle - cp_angle);

  if (cp_dist > mobj->radius) {
    return -1;
  }

  return sqrt(center_dist * center_dist + cp_dist * cp_dist);
}

} // namespace math
