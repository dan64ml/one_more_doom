#include "plane_utils.h"

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

} // namespace math
