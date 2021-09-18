#ifndef BLOCK_MAP_H_
#define BLOCK_MAP_H_

#include <fstream>
#include <vector>
#include <list>

#include <unordered_set>

#include "world_types.h"
#include "objects/map_object.h"

namespace world {

using MobjRange = std::unordered_set<mobj::MapObject*>;
using LineRange = std::unordered_set<const world::Line*>;

class BlockMap {
 public:
  BlockMap() = default;

  void Load(std::ifstream& fin, int offset, int size, const std::vector<Line>& lines);

  // Add new mobj in appropriate block
  void AddMapObject(mobj::MapObject* mobj);
  // Delete mobj from the block. False if no such an object
  bool DeleteMapObject(mobj::MapObject* mobj);
  // Change mobj's position. False if there is no such an object in (mobj.x, mobj.y)
  bool MoveMapObject(mobj::MapObject* mobj, int new_x, int new_y);

  //MobjRange GetMapObjects(int left_x, int right_x, int top_y, int bottom_y);
  MobjRange GetMapObjects(const BBox& bbox) const;
  //LineRange GetLines(int left_x, int right_x, int top_y, int bottom_y);
  LineRange GetLines(const BBox& bbox) const;

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

  const int kBlockShift = 7;
};

} // namespace world

#endif  // BLOCK_MAP_H_