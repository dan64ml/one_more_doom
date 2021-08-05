#ifndef WORLD_TYPES_H_
#define WORLD_TYPES_H_

#include <string>
#include <list>

namespace world {

struct Sector {
  int floor_height;
  int ceiling_height;
  
  std::string floor_pic;
  std::string ceiling_pic;
  
  int light_level;
  int special;  // For damage floor, light and etc...
  int tag;
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

  int flags;
  int specials;   // Special Type
  int tag;

  // sides[1] == nullptr if a wall
  SideDef* sides[2];
};

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
  SideDef* side;

  int angle;    // Направление сегмента. Из v1. Восток - 0. Север - 16384. Юг - -16384.
  int offset;   // Offset: distance along linedef to start of seg
};

// Modified BSP tree leaf. Contains pointers to segments (world::Segment) that produced
// this subsector.
struct SubSector {
  std::vector<Segment*> segs;
  // Objects located in this subsector
  // std::list<MapObject*> mobjs;
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

} // namespace world

#endif  // WORLD_TYPES_H_
