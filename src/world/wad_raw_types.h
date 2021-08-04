#ifndef WAD_RAW_TYPES_H_
#define WAD_RAW_TYPES_H_

#include <cstdint>

namespace wad {

// WAD file header
struct WadHeader {
  char magic[4];    // Magic word, must be IWAD or PWAD
  int32_t num_directories;  // The number of lumps
  int32_t directory_offset; // The directory contains lumps
};

// All lumps are messed up. To find level's lumps looking for MAPXY marker lump
// Then read fixed number lumps (kLevelLumpsNumber)
struct WadDirectoryEntry {
  int32_t offset;   // offset to the lump
  int32_t size;     // size of the lump
  char name[8];     // name of the lump
};

struct WadVertex {
    int16_t x;
    int16_t y;
};

struct WadLineDef {
  int16_t v1;
  int16_t v2;
  int16_t flags;
  int16_t specials;   // Special Type
  int16_t tag;
  int16_t sidenum[2];	// WadSideDef's indices. There is only one side (wall) if sidenum[1] == -1
};

struct WadSideDef {
  int16_t texture_offset; // x offset
  int16_t row_offset;     // y offset
  char top_texture[8];
  char bottom_texture[8];
  char mid_texture[8];
  int16_t sector;
};

struct WadSector {
  int16_t floor_height;
  int16_t ceiling_height;
  char floor_pic[8];
  char ceiling_pic[8];
  int16_t light_level;
  int16_t special;  // For damage floor, light and etc...
  int16_t tag;
};

struct WadThing {
  int16_t x;
  int16_t y;
  // Degrees. 0 - east, 90 - north, 180 - west
  int16_t angle;
  int16_t type;
  int16_t option;
};

} // namespace wad

#endif  // WAD_RAW_TYPES_H_
