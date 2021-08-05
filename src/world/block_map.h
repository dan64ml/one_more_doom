#ifndef BLOCK_MAP_H_
#define BLOCK_MAP_H_

#include <fstream>
#include <vector>
#include <list>

#include "world_types.h"
#include "objects/map_object.h"

namespace world {

class BlockMap {
 public:
  BlockMap() = default;

  void Load(std::ifstream& fin, int offset, int size, const std::vector<Line>& lines);

 private:
  // Contains lines and items that can be affected by other objects
  // E.g. player CAN be, but missile can't
  struct BlockMapEntry {
    std::vector<const Line*> lines;
    std::list<mobj::MapObject*> mobjs;
  };

 private:
  // Origin offset
  int x_offset_;
  int y_offset_;
  // Blocks with lines and objects
  std::vector<std::vector<BlockMapEntry>> blocks_;
};

} // namespace world

#endif  // BLOCK_MAP_H_