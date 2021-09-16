#ifndef WORLD_UTILS_H_
#define WORLD_UTILS_H_

#include <tuple>

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


// Returns distance from {x1, y2} to intersection point with mobj. Returns -1 if there is
// no intersection. Mobj is supposed to be a sprite section.
double GetDistanceToIntersection(int x1, int y1, int x2, int y2, const mobj::MapObject* mobj);
// The same for lines
double GetDistanceToIntersection(int x1, int y1, int x2, int y2, const world::Line* line);

// Returns intersection point with mobj (if exist).
std::tuple<bool, double, double> GetMobjIntersection(int x1, int y1, int x2, int y2, const mobj::MapObject* mobj);

// Returns intersection point of two LINES (not segments) if it exists.
std::tuple<bool, double, double> GetLinesIntersection(int l1_x1, int l1_y1, int l1_x2, int l1_y2, 
                                                      int l2_x1, int l2_y1, int l2_x2, int l2_y2);
std::tuple<bool, double, double> GetLinesIntersection(int x1, int y1, int x2, int y2, const world::Line* line);

// Returns intersection point of two segments if it exists.
std::tuple<bool, double, double> GetSegmentsIntersection(int x1, int y1, int x2, int y2, const world::Line* line);
std::tuple<bool, double, double> GetSegmentsIntersection(int s1_x1, int s1_y1, int s1_x2, int s1_y2,
                                                         int s2_x1, int s2_y1, int s2_x2, int s2_y2);

} // namespace math

#endif  // WORLD_UTILS_H_