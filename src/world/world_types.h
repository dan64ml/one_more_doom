#ifndef WORLD_TYPES_H_
#define WORLD_TYPES_H_

#include <string>
#include <list>
#include <vector>
#include <variant>

#include "objects/map_object.h"

namespace world {

class World;

// Just boundary box
struct BBox {
  double left;
  double right;
  double top;
  double bottom;

  void AddPoint(double x, double y) {
    if (x < left) {
      left = x;
    } else if (x > right) {
      right = x;
    }
    if (y < bottom) {
      bottom = y;
    } else if (y > top) {
      top = y;
    }
  }
};

struct Sector {
  double floor_height;
  double ceiling_height;
  
  std::string floor_pic;
  std::string ceiling_pic;
  
  int light_level;
  int special;  // For damage floor, light and etc...
  int tag;

  World* world;

  BBox bbox;

  // Important stuff for monster's AI. They can hear shots and it points to the source.
  // TODO: Add sound propagation
  mobj::MapObject* sound_source = nullptr;

  // True if the sector has active StructureObject
  bool has_sobj;
  // Lines forming this sector
  std::vector<Line*> lines;
  // List of subsectors
  std::vector<SubSector*> subsecs;
};

struct SideDef {
  // add to the calculated texture column
  int texture_offset; // x offset
  // add to the calculated texture top
  int row_offset;     // y offset

  std::string top_texture;
  std::string bottom_texture;
  std::string mid_texture;

  Sector* sector;
};

struct Line {
  // Left and right vertices
  int x1;
  int y1;
  int x2;
  int y2;

  // See kLDFxxx flags
  uint32_t flags;
  int specials;   // Special Type
  int tag;

  BBox bbox;

  // sides[1] == nullptr if a wall
  SideDef* sides[2];
};

const uint32_t kLDFBlockEveryOne = 0x0001;
const uint32_t kLDFBlockMonsters = 0x0002;
const uint32_t kLDFTwoSided      = 0x0004;
const uint32_t kLDFUpperUnpegged = 0x0008;
const uint32_t kLDFLowerUnpegged = 0x0010;
const uint32_t kLDFSecret        = 0x0020;
const uint32_t kLDFBlockSound    = 0x0040;
const uint32_t kLDFNeverShowMap  = 0x0080;
const uint32_t kLDFAlwaysShowMap = 0x0100;

// BSP related types

// BSP splits lines into segments. We use these segments for rendering
// the scene in right order (BSP guarantee the order)
struct Segment {
  int x1;
  int y1;
  int x2;
  int y2;

  // The Line from which the segment was produced
  Line* linedef;
  // Related side of the line
  //SideDef* side;
  int side;

  int angle;    // ?????????????????????? ????????????????. ???? v1. ???????????? - 0. ?????????? - 16384. ???? - -16384.
  int offset;   // Offset: distance along linedef to start of seg
};

// Modified BSP tree leaf. Contains pointers to segments (world::Segment) that produced
// this subsector.
struct SubSector {
  std::vector<Segment*> segs;
  // Objects located in this subsector
  std::list<mobj::MapObject*> mobjs;
  // Parent sector
  Sector* sector;
};

// Helper types for lump operations

struct LumpPos {
  int position;
  int size;
};

struct RawVertex {
  int16_t x;
  int16_t y;
};

// Helper type 
struct IntersectedObject {
  double distance;

  double x;
  double y;

  std::variant<Line*, mobj::MapObject*> obj;
};

// For hitscan etc...
struct Opening {
  double coef_high_opening = 100.0 / 160;
  double coef_low_opening = 100.0 / 160;

  int view_line_z;

  // Ceiling and floor after last correction
  int high_z;
  int low_z;
};

} // namespace world

#endif  // WORLD_TYPES_H_
