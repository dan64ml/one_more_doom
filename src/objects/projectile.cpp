#include "projectile.h"

#include <cstdlib>

#include "world/world.h"

namespace mobj {

Projectile::Projectile(const wpn::ProjectileParams& param, const MapObject* parent) 
  : MapObject(id::mobjinfo[param.type]) {
  // TODO: spawn at the end of the barrel
  x = parent->x;
  y = parent->y;
  z = parent->z + param.spawn_height;

  //damage_ = param.damage;
  damage_ = id::mobjinfo[param.type].damage;
  speed_ = id::mobjinfo[param.type].speed;
  blast_damage_ = param.blast_damage;

  mom_x = speed_ * rend::BamCos(parent->angle);
  mom_y = speed_ * rend::BamSin(parent->angle);
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

bool Projectile::InfluenceObject(MapObject* obj) {
  // check z position
  if (obj->z > (z + height)) {
    return true;
  }
  if ((obj->z + obj->height) < z) {
    return true;
  }

  // TODO: it's possible some demons can't be hit by demon's projectiles??! Check it...

  if (!(obj->flags & MF_SHOOTABLE)) {
    // Don't cause damage
    return !(obj->flags & MF_SOLID);
  }

  int damage = damage_ * (rand() % 8 + 1);
  obj->CauseDamage(damage);

  // Hit only one object
  return false;
}

bool Projectile::ProcessLine(const world::Line* line) {
  if (line->sides[1] == nullptr) {
    return false;
  }

  // check portal's opening
  if (line->sides[1]->sector->floor_height > z) {
    return false;
  }
  if (line->sides[1]->sector->ceiling_height < (z + height)) {
    return false;
  }

  // TODO: Do I have to check any line's flags here???

  return true;
}

} // namespace wpn
