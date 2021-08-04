#ifndef WORLD_TYPES_H_
#define WORLD_TYPES_H_

#include <string>

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
