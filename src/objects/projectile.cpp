#include "projectile.h"

#include <cstdlib>

#include "world/world.h"
#include "utils/world_utils.h"
#include "renderer/plane_utils.h"

namespace mobj {

Projectile::Projectile(id::mobjtype_t type, MapObject* parent, rend::BamAngle vert_angle) 
  : MovingObject(type), parent_(parent) {
  // Spawn at the end of the barrel
  int dx = parent->radius * rend::BamCos(parent->angle);
  int dy = parent->radius * rend::BamSin(parent->angle);

  x = parent->x + dx;
  y = parent->y + dy;
  z = parent->z + kBarrelHeight;

  damage_ = id::mobjinfo[type].damage;
  blast_damage_ = (type == id::MT_ROCKET) ? kRocketBlastDamage : 0;
  
  angle = parent->angle;

  mom_x = speed * rend::BamCos(angle);
  mom_y = speed * rend::BamSin(angle);
  mom_z = speed * rend::BamSin(vert_angle);
}

Projectile::Projectile(id::mobjtype_t type, MapObject* source, MapObject* target)
  : MovingObject(type), parent_(source) {
  target_ = target;
  x = source->x;
  y = source->y;
  z = source->z + kProjectileSpawnHeight;

  angle = rend::CalcAngle(source->x, source->y, target->x, target->y);

  mom_x = speed * rend::BamCos(angle);
  mom_y = speed * rend::BamSin(angle);

  double dist = rend::SegmentLength(source->x, source->y, target_->x, target_->y);
  mom_z = speed * (target_->z - source->z) / dist;

  damage_ = id::mobjinfo[type].damage;
  blast_damage_ = (type == id::MT_ROCKET) ? kRocketBlastDamage : 0;
}

bool Projectile::TickTime() {
  if (hit_sky_) {
    return false;
  }

  if (!met_obstacle) {
    return mobj::MovingObject::TickTime();
  } else {
    return fsm_.Tick(this);
  }
}

bool Projectile::RunIntoAction(double new_x, double new_y) {
  if (hit_sky_) {
    return false;
  }

  met_obstacle = true;

  fsm_.ToDeathState(this);

  if (blast_damage_ != 0) {
    //world_->DoBlastDamage(blast_damage_, x, y);
    world_->DoBlastDamage(blast_damage_, this);
  }

  return false;
}

bool Projectile::InfluenceObject(MapObject* obj) {
  if (obj == parent_) {
    return true;
  }
//  if (obj->mobj_type == id::MT_PLAYER) {
//    // Hack to prevent crushing into the player. Bad for multiplayer...
//    return true;
//  }
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
  obj->CauseDamage(damage, this, parent_);

  // Hit only one object
  return false;
}

bool Projectile::ProcessLine(world::Line* line) {
  if (line->sides[1] == nullptr) {
    if (line->sides[0]->sector->ceiling_height < z &&
        line->sides[0]->sector->ceiling_pic == "F_SKY1") {
      hit_sky_ = true;
    }
    return false;
  }

  if (line->sides[0]->sector->ceiling_pic == "F_SKY1" &&
      line->sides[1]->sector->ceiling_pic == "F_SKY1" &&
      line->sides[0]->sector->ceiling_height > line->sides[1]->sector->ceiling_height &&
      line->sides[1]->sector->ceiling_height < (z + height)) {
    // It's the sky
    hit_sky_ = true;
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

void Projectile::ZMove() {
  z += mom_z;
}

void Projectile::CallStateFunction(id::FuncId foo_id) {
  switch (foo_id)
  {
    case id::A_NULL:
      break;
    case id::A_BFGSpray:
      BFGSpray();
      break;

    default:
      break;
  }
}

void Projectile::BFGSpray() {
  const auto player = world_->GetPlayer();

  for (int i = 0; i < 40; ++i) {
    rend::BamAngle ang = angle - rend::kBamAngle45 + rend::kBamAngle90 / 40 * i;

    mobj::MapObject* target = world_->GetTarget(player->x, player->y, player->z + mobj::kWeaponHeight, ang, 1024);
    if (target) {
      auto [vx, vy] = math::ShiftToCenter(player->x, player->y, target->x, target->y, 2);

      world_->SpawnBFGExplode(vx, vy, target->z + target->height / 4);

      // Keep original behavior
      int damage = 0;
      for (int j = 0; j < 15; ++j) {
        damage += rand() % 7 + 1;
      }
      target->CauseDamage(damage, player, player);
    }
  }
}

} // namespace wpn
