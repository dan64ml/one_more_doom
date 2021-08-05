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

// BSP tree types
  
const uint16_t kNfSubsector = 0x8000;

// BSP tree node
struct WadBspNode {
  // Partition line
  int16_t x;
  int16_t y;
  int16_t dx;
  int16_t dy;

  // Bounding boxes. Right/left.
  int16_t bbox[2][4];

  // If bit kNfSubsector set - index of leaf (BspSubSector). Right/left.
  // Else - another BspNode index
  uint16_t children[2];
};

// BSP tree's leaf. Don't forget to remove the kNfSubsector bit in index
// It's a convex polygon and a set of BspSegments defines it.
struct WadBspSubSector {
  int16_t numsegs;
  // First BspSegment's index. Segments are stored sequentially
  int16_t firstseg;
};

// SEGS. А это те самые сегменты на которые указывают листы дерева
// Для одностороннего LineDef`а изначально создается один сегмент,
// для двухстороннего - два. Далее они могут разбиваться partition 
// line`ами на части с созданием дополнительных вершин
struct WadBspSegment {
  int16_t v1;
  int16_t v2;
  int16_t angle;    // Направление сегмента. Из v1. Восток - 0. Север - 16384. Юг - -16384.
  int16_t linedef;	// Индекс MapLineDef`а из которого получился сегмент
  int16_t side;     // 0 - right side of the segment. It looks like our side of the seg will be returned first by BSP???
                    // Each portal produces at leat TWO segments (for left side and right side). It's the index of the side
  int16_t offset;   // Offset: distance along linedef to start of seg
};

// Headef of BLOCKMAP lump
struct WadBlockmapHeader {
  // origin of the grid. col = (x_pos - x) >> 7. row is the same
  int16_t x;
  int16_t y;
  int16_t columns;
  int16_t rows;
};

// Map items, THINGS lump
struct WadMapThing {
  int16_t x;
  int16_t y;
  // Degrees. 0 - east, 90 - north, 180 - west
  int16_t angle;
  int16_t type;
  int16_t option;
};

} // namespace wad

#endif  // WAD_RAW_TYPES_H_
