#ifndef FAST_BSP_H_
#define FAST_BSP_H_

#include <fstream>
#include <memory>
#include <vector>

namespace wad {

class FastBsp {
 public:
  FastBsp() = default;

  void LoadBsp(std::ifstream& fin, int offset, int size);

  int GetSubSectorIdx(int x, int y) const;

 private:
  // BSP tree node
  struct BspNode {
    // Partition line
    int16_t x;
    int16_t y;
    int16_t dx;
    int16_t dy;
    // Bounding boxes. Right/left.
    int16_t bbox[2][4];
    // If bit kNfSubsector set - index of leaf. Right/left.
    uint16_t children[2];
  };
  // Marker bit for leafs
  const uint16_t kNfSubsector = 0x8000;

 private:
  std::vector<BspNode> nodes_;

  // Defines the position of the view point (x, y) relatively the partition line. 
  // 0 - front(right), 1 - back(left)
  int DefineVpSide(int x, int y, int node_idx) const;
};

} // namespace wad

#endif  // FAST_BSP_H_