#ifndef PLANE_UTILS_H_
#define PLANE_UTILS_H_

#include "vec2d.h"

#include <cstdint>
#include <limits>
#include <tuple>

#include "world/world_types.h"
#include "bam.h"

namespace rend {

// Point with direction
struct DPoint {
  int x;
  int y;
  BamAngle angle;

  DPoint() : x(0), y(0), angle(kBamAngle0) {}
  template<class T>
  DPoint(T x, T y, BamAngle a) : x(x), y(y), angle(a) {}
};

// Returns angle between OX and the line
BamAngle CalcAngle(int from_x, int from_y, int to_x, int to_y);

// Return coordinates of visible segment's ends with angles from view point !relative to right
// border of FOV!
// If segment is invisible return {false, 0, 0}
std::tuple<bool, DPoint, DPoint> 
GetVisibleSegment(DPoint view_point, int x1, int y1, int x2, int y2, BamAngle h_fov = kBamAngle90);
std::tuple<bool, DPoint, DPoint> 
GetVisibleSegment(DPoint view_point, const world::Line* line, BamAngle h_fov = kBamAngle90);

// Calculate line that goes through two points. Return normal vector and shift along the vector.
std::pair<math::Vec2d, double> CreateLine(int x1, int y1, int x2, int y2);
std::pair<math::Vec2d, double> CreateLine(const world::Line* line);
// Calculate line by a point and direction. Return normal vector and shift along the vector.
std::pair<math::Vec2d, double> CreateLine(int x, int y, BamAngle direction);

// Calculate the point of intersection of two lines.
// ATTENTION! It does not check for intersection!
std::pair<double, double> CalcIntersectionPoint(math::Vec2d n1, double d1, math::Vec2d n2, double d2);
// ???
bool FindIntersectionPoint(double& cx, double& cy, math::Vec2d n1, double d1, math::Vec2d n2, double d2, double eps = .0001);

// 0 - right side, 1 - left
int DefineVpSide(int x1, int y1, int x2, int y2, int vp_x, int vp_y);
int DefineVpSide(const world::Line* line, int vp_x, int vp_y);

// Calculate intersection of two ranges. {0, -1} if there is no intersection.
std::pair<int, int> CreateIntersection(int r1_left, int r1_right, int r2_left, int r2_right);

// Calculate distance between two points
double SegmentLength(double x1, double y1, double x2, double y2);
double SegmentLength(DPoint p1, DPoint p2);

} // namespace rend

#endif  // PLANE_UTILS_H_