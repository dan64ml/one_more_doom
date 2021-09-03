#include "projectile.h"

namespace mobj {

Projectile::Projectile(const wpn::ProjectileParams& param) 
  : MapObject(id::mobjinfo[param.type]) {

}

Projectile::Projectile(const wpn::ProjectileParams& param, const MapObject* parent) 
  : MapObject(id::mobjinfo[param.type]) {
  x = parent->x;
  y = parent->y;
  z = parent->z + param.spawn_height;

  mom_x = param.speed * rend::BamCos(parent->angle);
  mom_y = param.speed * rend::BamSin(parent->angle);
  mom_z = 0;
}

bool Projectile::TickTime() {
  return mobj::MapObject::TickTime();
}

bool Projectile::RunIntoAction() {
  fsm_.ToDeathState();
  return false;
}

} // namespace wpn
