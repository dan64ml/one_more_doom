#ifndef WORLD_UTILS_H_
#define WORLD_UTILS_H_

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

bool IsLineIntersectMobj(int x1, int y1, int x2, int y2, const mobj::MapObject* mobj);

// Returns distance from {x1, y2} to intersection point with mobj. Returns -1 if there is
// no intersection. Mobj is supposed to be a sprite section.
double GetDistanceToIntersection(int x1, int y1, int x2, int y2, const mobj::MapObject* mobj);
// The same for lines
double GetDistanceToIntersection(int x1, int y1, int x2, int y2, const world::Line* line);

} // namespace math

#endif  // WORLD_UTILS_H_