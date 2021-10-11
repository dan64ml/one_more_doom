#ifndef SOBJ_UTILS_H_
#define SOBJ_UTILS_H_

#include "sobj_types.h"

#include "world/world_types.h"

namespace sobj {

// Moves sector's ceiling in the given direction with speed until reach the dest.
// If crush == true cause damage
MoveResult MoveCeiling(world::Sector* sec, double speed, double dest, bool crush, MoveDirection dir);

} // namespace sobj

#endif  // SOBJ_UTILS_H_
