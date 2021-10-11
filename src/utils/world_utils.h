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

// "Shift" point closer to the center. 
std::pair<int, int> ShiftToCenter(int cx, int cy, int x, int y, int shift = 1);

// Corrects opening using Line. Returns false if opening is closed
bool CorrectOpening(world::Opening& op, const world::Line* line, double dist);

// Returns height of opening for a portal. 0 for a wall.
int GetOpenRange(const world::Line* line);

// Find lowest ceiling around the sector
double GetLowestCeilingHeight(const world::Sector* sec);
// Looks for the highest floor level
double GetHighestFloorHeight(const world::Sector* sec);
// Looks for the lowest floor level
double GetLowestFloorHeight(const world::Sector* sec);
// Looks for the sector that has minimal floor height larger then current_height.
// Returns current_height if it's the highest.
double GetNextHighestFloorHeight(const world::Sector* sec, int current_height);
//
double GetHighestCeilingHeight(const world::Sector* sec);

// Helper. Returns opposite sector for the line (if it exists)
world::Sector* GetOppositeSector(const world::Sector* sec, const world::Line* line);

} // namespace math

#endif  // WORLD_UTILS_H_