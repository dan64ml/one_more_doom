#ifndef GRAPHICS_TYPES_H_
#define GRAPHICS_TYPES_H_

#include <string>
#include <vector>

namespace graph {

struct MapPatchRaw {
  int16_t origin_x;   // horizontal offset of the patch relative to the upper-left of the texture
  int16_t origin_y;   // vertical offset of the patch relative to the upper-left of the texture
  int16_t patch;      // patch number (as listed in PNAMES) to draw
  int16_t step_dir;   // ? Unused
  int16_t color_map;  // ? Unused
};

// Start point to get texture pixels. Contains an array of MapPatchRaw, that cover texture like tile
struct MapTextureRaw {
  char    name[8];
  int32_t masked;
  int16_t width;  // total width of the map texture
  int16_t height; // total height of the map texture
  int32_t column_directory; // OBSOLETE
  int16_t patch_count;  // The number of map patches that make up this map texture
  MapPatchRaw patches[1]; // Array with the map patch structures for this texture
};

struct PatchRaw {
  int16_t width;
  int16_t height;
  int16_t left_offset;
  int16_t top_offset;
  uint32_t columnofs[8];
};

// posts are runs of non masked source pixels
// post_t
struct PostRaw {
  uint8_t top_delta;  // -1 is the last post in a column
  uint8_t lenght;   // length data bytes follow
};
// column_t is a list of 0 or more post_t, (uint8_t)-1 terminated
// typedef post_t column_t;


// Image with transparent pixels
struct PixelPicture {
  int width;
  int height;
  std::vector<uint16_t> pixels;
};

// Image without transparent pixels
struct FlatPicture {
  int width;
  int height;
  std::vector<uint8_t> pixels;
};

const uint16_t kTransparentColor = 0x100;

} // namespace graph

#endif  // GRAPHICS_TYPES_H_