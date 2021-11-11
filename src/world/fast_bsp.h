#ifndef FAST_BSP_H_
#define FAST_BSP_H_

#include <fstream>
#include <memory>
#include <vector>

#include "world_types.h"
#include "bsp_iterator.h"

namespace wad {

class FastBsp {
 public:
  FastBsp() = default;

  void LoadBsp(std::ifstream& fin, int offset, int size);
  void SetSubSectors(const std::vector<world::SubSector>* ss) { ss_ = ss; }

  int GetSubSectorIdx(double x, double y) const;

  void SetViewPoint(double x, double y) const { vp_x_ = x; vp_y_ = y; }

  world::BspIterator begin() const;
  world::BspIterator end() const;
  world::BspIterator cbegin() const;
  world::BspIterator cend() const;


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
  const static uint16_t kNfSubsector = 0x8000;

 private:
  std::vector<BspNode> nodes_;

  const std::vector<world::SubSector>* ss_ = nullptr;
  mutable double vp_x_ = 0;
  mutable double vp_y_ = 0;

  // Defines the position of the view point (x, y) relatively the partition line. 
  // 0 - front(right), 1 - back(left)
  int DefineVpSide(double x, double y, int node_idx) const;

  friend class world::BspIterator;
};

} // namespace wad

#endif  // FAST_BSP_H_