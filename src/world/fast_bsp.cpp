#include "fast_bsp.h"

#include "wad_utils.h"
#include "bsp_iterator.h"

namespace wad {

world::BspIterator FastBsp::begin() const {
  return world::BspIterator(this);
}

world::BspIterator FastBsp::end() const {
  return world::BspIterator();
}

world::BspIterator FastBsp::cbegin() const {
  return begin();
}

world::BspIterator FastBsp::cend() const {
  return end();
}

void FastBsp::LoadBsp(std::ifstream& fin, int offset, int size) {
  int node_number = size / sizeof(BspNode);
  nodes_.resize(node_number);

  fin.seekg(offset);
  fin.read(reinterpret_cast<char*>(nodes_.data()), size);
}

int FastBsp::GetSubSectorIdx(int x, int y) const {
  int node_num = nodes_.size() - 1;

  while (!(node_num & kNfSubsector)) {
    int side = DefineVpSide(x, y, node_num);
    node_num = nodes_[node_num].children[side];
  }

  return node_num & ~kNfSubsector;
}

int FastBsp::DefineVpSide(int x, int y, int node_idx) const {
  const BspNode& node = nodes_[node_idx];

  if (!node.dx) {
    if (x <= node.x) {
      return node.dy > 0;
    } else {
      return node.dy < 0;
    }
  }

  if (!node.dy) {
    if (y <= node.y) {
      return node.dx < 0;
    } else {
      return node.dx > 0;
    }
  }

  double dvp_x = x - node.x;
  double dvp_y = y - node.y;
  double area = node.dx * dvp_y - dvp_x * node.dy;

  return (area > 0);
}

} // namespace wad