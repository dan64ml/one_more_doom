#include "sobj_utils.h"

#include <cassert>

namespace sobj {

MoveResult MoveCeiling(world::Sector* sec, double speed, double dest, bool crush, MoveDirection dir) {
  double new_pos;
  
  switch (dir)
  {
    case MoveDirection::kDown:
      new_pos = sec->ceiling_height - speed;
      break;
    
    default:
      assert(false);
      break;
    }
}

} // namespace sobj
