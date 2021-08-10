#ifndef PLANE_UTILS_H_
#define PLANE_UTILS_H_

#include "vec2d.h"

#include <cstdint>
#include <limits>
#include <tuple>

#include "world/world_types.h"

namespace rend {

// Pi
const double kPi = 3.1415926;

// 0 == East (OX+), 16384 == North (OY+).
using BamAngle = uint16_t;
const BamAngle kBamAngle0 = 0;
const BamAngle kBamAngle45 = std::numeric_limits<BamAngle>::max() / 8;
const BamAngle kBamAngle90 = std::numeric_limits<BamAngle>::max() / 4;
const BamAngle kBamAngle135 = (std::numeric_limits<BamAngle>::max() / 8) * 3;
const BamAngle kBamAngle180 = std::numeric_limits<BamAngle>::max() / 2;
const BamAngle kBamAngle360 = std::numeric_limits<BamAngle>::max();


// Point with direction
struct DPoint {
  int x;
  int y;
  BamAngle angle;

  DPoint() : x(0), y(0), angle(kBamAngle0) {}
  template<class T>
  DPoint(T x, T y, BamAngle a) : x(x), y(y), angle(a) {}
};

// Convert degrees to BamAngle
BamAngle DegreesToBam(int angle);

// Calculate angle between line segment and OX axis
BamAngle CalcAngle(int from_x, int from_y, int to_x, int to_y);

// Triginometric functions. May be realized like a lookup tables.
double BamCos(BamAngle a);
double BamSin(BamAngle a);
double BamTan(BamAngle a);

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

// 0 - right side, 1 - left
int DefineVpSide(int x1, int y1, int x2, int y2, int vp_x, int vp_y);
int DefineVpSide(const world::Line* line, int vp_x, int vp_y);

// Calculate intersection of two renges. {0, -1} if there is no intersection.
std::pair<int, int> CreateIntersection(int r1_left, int r1_right, int r2_left, int r2_right);

} // namespace rend

#endif  // PLANE_UTILS_H_