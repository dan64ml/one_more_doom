#ifndef PLANE_UTILS_H_
#define PLANE_UTILS_H_

#include "world/world_types.h"

namespace math {

enum ObjPosition : int {
  kCross = -1,
  // Right side == front side of a line
  kRightSide = 0,
  // Left side == back side of a line
  kLeftSide = 1,
};

// Finds point position relative to line
ObjPosition LinePointPosition(const world::Line* line, int x, int y);
// Finds position of the BBox relative to the line
ObjPosition LineBBoxPosition(const world::Line* line, const world::BBox* bbox);

} // namespace math

#endif  // PLANE_UTILS_H_