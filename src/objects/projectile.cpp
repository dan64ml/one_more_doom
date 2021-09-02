#include "projectile.h"

namespace mobj {

Projectile::Projectile(const wpn::ProjectileParams& param) 
  : MapObject(id::mobjinfo[param.type]) {

}

bool Projectile::TickTime() {
  return mobj::MapObject::TickTime();
}

} // namespace wpn
