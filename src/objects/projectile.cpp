#include "projectile.h"

#include "world/world.h"

namespace mobj {

Projectile::Projectile(const wpn::ProjectileParams& param, const MapObject* parent) 
  : MapObject(id::mobjinfo[param.type]) {
  x = parent->x;
  y = parent->y;
  z = parent->z + param.spawn_height;

  damage_ = param.damage;
  blast_damage_ = param.blast_damage;

  mom_x = param.speed * rend::BamCos(parent->angle);
  mom_y = param.speed * rend::BamSin(parent->angle);
  mom_z = 0;
}

bool Projectile::TickTime() {
  if (!met_obstacle) {
    return mobj::MapObject::TickTime();
  } else {
    return fsm_.Tick();
  }
}

bool Projectile::RunIntoAction() {
  met_obstacle = true;

  fsm_.ToDeathState();

  if (blast_damage_ != 0) {
    world_->DoBlastDamage(blast_damage_, x, y);
  }
  
  return false;
}

} // namespace wpn
