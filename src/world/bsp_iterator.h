#ifndef BSP_H_
#define BSP_H_

#include <stack>

#include "world_types.h"

namespace wad {
  class FastBsp;
}

namespace world {

class BspIterator {
 public:
  BspIterator() = default;
  BspIterator(const wad::FastBsp* fbsp);

  bool operator==(const BspIterator& rhs) const;
  bool operator!=(const BspIterator& rhs) const;
  const SubSector* operator*() const;
  const BspIterator& operator++();

 private:
  const wad::FastBsp* fbsp_;
  std::stack<uint16_t> S_;
  int current_leaf = 0;

  void FindNextNode();
};

} // namespace world

#endif  // BSP_H_
