#include "projectile.h"

namespace wpn {

bool Projectile::TickTime() {
  return mobj::MapObject::TickTime();
}

} // namespace wpn
